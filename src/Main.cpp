///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// See https://www.gnu.org/licenses/gpl-2.0-standalone.html for details!
///////////////////////////////////////////////////////////////////////////////

//Internal
#include "Compat.h"
#include "Utils.h"
#include "Version.h"
#include "URL.h"
#include "Params.h"
#include "Client_FTP.h"
#include "Client_HTTP.h"
#include "Sink_File.h"
#include "Sink_StdOut.h"
#include "Sink_Null.h"
#include "Timer.h"
#include "Average.h"
#include "Thread.h"

//Win32
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinINet.h>

//CRT
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <algorithm>

//Externals
namespace Zero
{
	extern Sync::Signal g_sigUserAbort;
}

//=============================================================================
// HELPER MACROS
//=============================================================================

#define ABORTED_BY_USER (Zero::g_sigUserAbort.get())

#define TRIGGER_SYSTEM_SOUND(X,Y) do \
{ \
	if((X))  \
	{ \
		Utils::trigger_system_sound((Y)); \
	} \
} \
while(0)

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

static const wchar_t *const UPDATE_INFO = L"Check http://muldersoft.com/ or https://github.com/lordmulder/ for updates!\n";

static std::string stdin_get_line(void)
{
	std::string line;
	std::getline(std::cin, line);
	return line;
}

static std::wstring build_version_string(void)
{
	std::wostringstream str;
	if(VERSION_PATCH > 0)
	{
		str << std::setw(0) << VERSION_MAJOR << L'.' << std::setfill(L'0') << std::setw(2) << VERSION_MINOR << L'-' << std::setw(0) << VERSION_PATCH;
	}
	else
	{
		str << std::setw(0) << VERSION_MAJOR << L'.' << std::setfill(L'0') << std::setw(2) << VERSION_MINOR;
	}
	return str.str();
}

static void print_logo(void)
{
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr << L"\nINetGet v" << build_version_string() << L" - Lightweight command-line front-end to WinINet\n"
		<< L"Copyright (c) " << &BUILD_DATE[7] << L" LoRd_MuldeR <mulder2@gmx.de>. Some rights reserved.\n"
		<< L"Built on " << BUILD_DATE << L" at " << BUILD_TIME << L", " << BUILD_COMP << L", Win-" << BUILD_ARCH << L", " << BUILD_CONF << L'\n' << std::endl;
	std::wcout.flags(stateBackup);

	const time_t build_time = Utils::decode_date_str(BUILD_DATE);
	if(build_time > 0)
	{
		const time_t now = time(NULL);
		if((now > build_time) && ((now - build_time) >= 31556926i64))
		{
			std::wcerr << L"NOTE: This binary is more than a year old, there probably is a new version.\n" << UPDATE_INFO << std::endl;
		}
	}
}

static void print_help_screen(void)
{
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr << UPDATE_INFO
		<< L'\n'
		<< L"Usage:\n"
		<< L"  INetGet.exe [options] <source_addr> <output_file>\n"
		<< L'\n'
		<< L"Required:\n"
		<< L"  <source_addr> : Specifies the source internet address (URL)\n"
		<< L"  <output_file> : Specifies the output file, you can specify \"-\" for STDOUT\n"
		<< L'\n'
		<< L"Optional:\n"
		<< L"  --verb=<verb>   : Specify the HTTP method (verb) to be used, default is GET\n"
		<< L"  --data=<data>   : Append data to request, in 'x-www-form-urlencoded' format\n"
		<< L"  --no-proxy      : Don't use proxy server for address resolution\n"
		<< L"  --agent=<str>   : Overwrite the default 'user agent' string used by INetGet\n"
		<< L"  --no-redir      : Disable automatic redirection, enabled by default\n"
		<< L"  --range-off=<n> : Set the offset (start) of the byte range to be downloaded\n"
		<< L"  --range-end=<n> : Set the end of the byte range to be downloaded\n"
		<< L"  --insecure      : Don't fail, if server certificate is invalid (HTTPS only)\n"
		<< L"  --refer=<url>   : Include the given 'referrer' address in the request\n"
		<< L"  --notify        : Trigger a system sound when the download completed/failed\n"
		<< L"  --time-cn=<n>   : Specifies the connection timeout, in seconds\n"
		<< L"  --time-rc=<n>   : Specifies the receive timeout, in seconds\n"
		<< L"  --timeout=<n>   : Specifies the connection & receive timeouts, in seconds\n"
		<< L"  --retry=<n>     : Specifies the max. number of connection attempts\n"
		<< L"  --no-retry      : Do not retry, if the connection failed (i.e. '--retry=0')\n"
		<< L"  --force-crl     : Make the connection fail, if CRL could *not* be retrieved\n"
		<< L"  --set-ftime     : Set the file's Creation/LastWrite time to 'Last-Modified'\n"
		<< L"  --update        : Update (replace) local file, iff server has newer version\n"
		<< L"  --keep-failed   : Keep the incomplete output file, when download has failed\n"
		<< L"  --config=<cf>   : Read INetGet options from specified configuration file(s)\n"
		<< L"  --help          : Show this help screen\n"
		<< L"  --slunk         : Enable slunk mode, this is intended for kendo master only\n"
		<< L"  --verbose       : Enable detailed diagnostic output (for debugging)\n"
		<< L'\n'
		<< L"Examples:\n"
		<< L"  INetGet.exe http://www.warr.org/buckethead.html output.html\n"
		<< L"  INetGet.exe --verb=POST --data=\"foo=bar\" http://localhost/form.php result\n"
		<< std::endl;
	std::wcout.flags(stateBackup);
}

static bool create_client(std::unique_ptr<AbstractClient> &client, AbstractListener &listener, const int16_t scheme_id, const Params &params)
{
	switch(scheme_id)
	{
	case INTERNET_SCHEME_FTP:
		client.reset(new FtpClient(Zero::g_sigUserAbort, params.getDisableProxy(), params.getUserAgent(), params.getVerboseMode()));
		break;
	case INTERNET_SCHEME_HTTP:
	case INTERNET_SCHEME_HTTPS:
		client.reset(new HttpClient(Zero::g_sigUserAbort, params.getDisableProxy(), params.getUserAgent(), params.getDisableRedir(), params.getRangeStart(), params.getRangeEnd(), params.getInsecure(), params.getForceCrl(), params.getTimeoutCon(), params.getTimeoutRcv(), params.getRetryCount(), params.getVerboseMode()));
		break;
	default:
		client.reset();
		break;
	}

	if(client)
	{
		client->add_listener(listener);
		return true;
	}

	return false;
}

static bool create_sink(std::unique_ptr<AbstractSink> &sink, const std::wstring fileName, const uint64_t &timestamp, const bool &keep_failed)
{
	if(_wcsicmp(fileName.c_str(), L"-") == 0)
	{
		sink.reset(new StdOutSink());
	}
	else if(_wcsicmp(fileName.c_str(), L"NUL") == 0)
	{
		sink.reset(new NullSink());
	}
	else
	{
		sink.reset(new FileSink(fileName, timestamp, keep_failed));
	}

	return sink ? sink->open() : false;
}

static void print_response_info(const uint32_t &status_code, const uint64_t &file_size,	const uint64_t &time_stamp, const std::wstring &content_type, const std::wstring &content_encd)
{
	static const wchar_t *const UNSPECIFIED = L"<N/A>";

	std::wcerr << L"--> HTTP Status code : " << status_code << L" [" << Utils::status_to_string(status_code) << "]\n";
	std::wcerr << L"--> Content type     : " << (content_type.empty() ? UNSPECIFIED : content_type) << L'\n';
	std::wcerr << L"--> Content encoding : " << (content_encd.empty() ? UNSPECIFIED : content_encd) << L'\n';
	std::wcerr << L"--> Content length   : " << ((file_size == AbstractClient::SIZE_UNKNOWN) ? UNSPECIFIED : std::to_wstring(file_size)) << L" Byte\n";
	std::wcerr << L"--> Last modified TS : " << ((time_stamp == AbstractClient::TIME_UNKNOWN) ? UNSPECIFIED : Utils::timestamp_to_str(time_stamp)) << L'\n';
	std::wcerr << std::endl;
}

//=============================================================================
// PRINT PROGRESS
//=============================================================================

typedef struct
{
	uint8_t spinner_index;
	uint8_t update_counter;
	double time_left;
	double current_rate;
	uint64_t total_bytes_last;
}
progress_t;

static inline void print_progress(const std::wstring url_string, uint64_t total_bytes, const uint64_t &file_size, Average &rate_estimate, Timer &timer_rate, progress_t &context)
{
	static const wchar_t SPINNER[4] = { L'-', L'\\', L'|', L'/' };
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr << std::setprecision(1) << std::fixed << std::setw(0) << L"\r[" << SPINNER[(context.spinner_index++) & 3] << L"] ";

	if(++context.update_counter >= 4)
	{
		context.current_rate = rate_estimate.update(double(total_bytes - context.total_bytes_last) / timer_rate.query());
		timer_rate.reset();
		context.total_bytes_last = total_bytes, context.update_counter = 0;
	}

	if(file_size != AbstractClient::SIZE_UNKNOWN)
	{
		const double percent = (file_size > 0.0) ? (100.0 * std::min(1.0, double(total_bytes) / double(file_size))) : 100.0;
		if(context.current_rate >= 0.0)
		{
			if(context.current_rate > 0.0)
			{
				const double eta_estimate = (total_bytes < file_size) ? (double(file_size - total_bytes) / context.current_rate) : 0.0;
				context.time_left = (context.time_left >= 0.0) ? ((context.time_left * 0.666) + (eta_estimate * 0.334)) : eta_estimate;
				if(context.time_left > 3)
				{
					std::wcerr << percent << L"% of " << Utils::nbytes_to_string(double(file_size)) << L" received, " << Utils::nbytes_to_string(context.current_rate) << L"/s, " << Utils::second_to_string(context.time_left) << L" remaining...";
				}
				else
				{
					std::wcerr << percent << L"% of " << Utils::nbytes_to_string(double(file_size)) << L" received, " << Utils::nbytes_to_string(context.current_rate) << L"/s, almost finished...";
				}
			}
			else
			{
				std::wcerr << percent << L"% of " << Utils::nbytes_to_string(double(file_size)) << L" received, " << Utils::nbytes_to_string(context.current_rate) << L"/s, please stand by...";
			}
		}
		else
		{
			std::wcerr << percent << L"% of " << Utils::nbytes_to_string(double(file_size)) << L" received, please stand by...";
		}

		std::wostringstream title;
		title << std::setprecision(1) << std::fixed << std::setw(0) << L"INetGet [" << percent << L"% of " << Utils::nbytes_to_string(double(file_size)) << L"] - " << url_string;
		Utils::set_console_title(title.str());
	}
	else
	{
		if(context.current_rate >= 0.0)
		{
			std::wcerr << Utils::nbytes_to_string(double(total_bytes)) << L" received, " << Utils::nbytes_to_string(context.current_rate) << L"/s, please stand by...";
		}
		else
		{
			std::wcerr << Utils::nbytes_to_string(double(total_bytes)) << L" received, please stand by...";
		}

		std::wostringstream title;
		title << L"INetGet [" << Utils::nbytes_to_string(double(total_bytes)) << L"] - " << url_string;
		Utils::set_console_title(title.str());
	}

	std::wcerr << L"    " << std::flush;
	std::wcout.flags(stateBackup);
}

//=============================================================================
// STATUS LISTENER
//=============================================================================

class StatusListener : public AbstractListener
{
protected:
	virtual void onMessage(const std::wstring message)
	{
		Sync::Locker locker(m_mutex);
		if(!ABORTED_BY_USER)
		{
			std::wcerr << L"--> " << message << std::endl;
		}
	}
private:
	Sync::Mutex m_mutex;
};

//=============================================================================
// CONNECTOR THREAD
//=============================================================================

class ConnectorThread : public Thread
{
public:
	ConnectorThread(AbstractClient *const client, const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer, const uint64_t &timestamp)
	:
		m_client(client), m_verb(verb), m_url(url), m_post_data(post_data), m_referrer(referrer), m_timestamp(timestamp)
	{
		m_priority.set(3);
	}
	
	static const uint32_t CONNECTION_COMPLETE = 0;
	static const uint32_t CONNECTION_ERR_INET = 1;
	static const uint32_t CONNECTION_ERR_ABRT = 3;

protected:
	virtual uint32_t main(void)
	{
		if(!m_client->open(m_verb, m_url, m_post_data, m_referrer, m_timestamp))
		{
			set_error_text(m_client->get_error_text());
			return CONNECTION_ERR_INET;
		}

		return is_stopped() ? CONNECTION_ERR_ABRT : CONNECTION_COMPLETE;
	}

private:
	AbstractClient *const m_client;

	const http_verb_t &m_verb;
	const URL &m_url;
	const std::string &m_post_data;
	const std::wstring &m_referrer;
	const uint64_t &m_timestamp;
};

//=============================================================================
// TRANSFER THREAD
//=============================================================================

class TransferThread : public Thread
{
public:
	TransferThread(AbstractSink *const sink, AbstractClient *const client)
	:
		m_sink(sink),
		m_client(client),
		m_transferred_bytes(0ui64)
	{
		m_priority.set(3);
	}

	uint64_t get_transferred_bytes(void)
	{
		return m_transferred_bytes.get();
	}

	static const uint32_t TRANSFER_COMPLETE = 0;
	static const uint32_t TRANSFER_ERR_INET = 1;
	static const uint32_t TRANSFER_ERR_SINK = 2;
	static const uint32_t TRANSFER_ERR_ABRT = 3;

protected:
	virtual uint32_t main(void)
	{
		bool eof_flag = false, abort_flag = false;

		while(!(eof_flag || (abort_flag = is_stopped())))
		{
			size_t bytes_read = 0;
			if(!m_client->read_data(m_buffer, BUFF_SIZE, bytes_read, eof_flag))
			{
				set_error_text(m_client->get_error_text());
				return TRANSFER_ERR_INET;
			}

			if(bytes_read > 0)
			{
				m_transferred_bytes.add(bytes_read);
				if(!(abort_flag = is_stopped()))
				{
					if(!m_sink->write(m_buffer, bytes_read))
					{
						return TRANSFER_ERR_SINK;
					}
				}
			}
		}

		return abort_flag ? TRANSFER_ERR_ABRT : TRANSFER_COMPLETE;
	}

private:
	AbstractSink *const m_sink;
	AbstractClient *const m_client;

	static const size_t BUFF_SIZE = 8192;
	uint8_t m_buffer[BUFF_SIZE];
	Sync::Interlocked<uint64_t> m_transferred_bytes;
};

//=============================================================================
// PROCESS
//=============================================================================

static int transfer_file(AbstractClient *const client, const std::wstring &url_string, const uint64_t &file_size, const uint64_t &timestamp, const std::wstring &outFileName, const bool &alert, const bool &keep_failed)
{
	//Open output file
	std::unique_ptr<AbstractSink> sink;
	if(!create_sink(sink, outFileName, timestamp, keep_failed))
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << L"ERROR: Failed to open the sink, unable to download file!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Initialize local variables
	Average rate_estimate(32);
	progress_t progress = { 0, 0, -1.0, -1.0, 0ui64 };
	Timer timer_total, timer_rate;

	//Start thread
	std::unique_ptr<TransferThread> transfer_thread (new TransferThread(sink.get(), client));
	if(!transfer_thread->start())
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << L"ERROR: Failed to start the file transfer thread!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Print progress
	std::wcerr << L"Download in progress:" << std::endl;
	print_progress(url_string, transfer_thread->get_transferred_bytes(), file_size, rate_estimate, timer_rate, progress);

	while(!transfer_thread->join(Zero::g_sigUserAbort, 250))
	{
		//Check for user abort
		if(ABORTED_BY_USER)
		{
			std::wcerr << L"\b\b\babort!\n"<< std::endl;
			transfer_thread->stop(1250, true);
			std::wcerr << L"SIGINT: Operation aborted by the user !!!\n" << std::endl;
			return EXIT_FAILURE;
		}

		//Update progress
		if(!ABORTED_BY_USER)
		{
			print_progress(url_string, transfer_thread->get_transferred_bytes(), file_size, rate_estimate, timer_rate, progress);
		}
	}

	//Check thread result
	const uint32_t thread_result = transfer_thread->get_result();
	if(thread_result != TransferThread::TRANSFER_COMPLETE)
	{
		std::wcerr << L"\b\b\bfailed\n" << std::endl;
		const std::wstring error_text = transfer_thread->get_error_text();
		switch(thread_result)
		{
		case TransferThread::TRANSFER_ERR_INET:
			if(!error_text.empty())
			{
				std::wcerr << error_text << L'\n' << std::endl;
			}
			std::wcerr << L"ERROR: Failed to receive incoming data, download has failed!\n" << std::endl;
			break;
		case TransferThread::TRANSFER_ERR_SINK:
			std::wcerr << L"ERROR: Failed to write data to sink, download has failed!\n" << std::endl;
			break;
		case TransferThread::TRANSFER_ERR_ABRT:
			std::wcerr << L"ERROR: The operation has been aborted !!!\n" << std::endl;
			break;
		default:
			std::wcerr << L"ERROR: The operation failed for an unknwon reason!\n" << std::endl;
			break;
		}
		TRIGGER_SYSTEM_SOUND(alert, false);
		return EXIT_FAILURE;
	}

	//Finalize progress
	print_progress(url_string, transfer_thread->get_transferred_bytes(), file_size, rate_estimate, timer_rate, progress);
	std::wcerr << L"\b\b\bdone\n" << std::endl;

	//Compute average download rate
	const double total_time = timer_total.query();
	const double average_rate = double(transfer_thread->get_transferred_bytes()) / total_time;

	//Check for user abort
	if(ABORTED_BY_USER)
	{
		std::wcerr << L"SIGINT: Operation aborted by the user !!!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Flush and close the sink
	std::wcerr << L"Flushing output buffers... " << std::flush;
	sink->close(true);

	//Report total time and average download rate
	TRIGGER_SYSTEM_SOUND(alert, true);
	std::wcerr << L"done\n\nDownload completed in " << ((total_time >= 1.0) ? Utils::second_to_string(total_time) : L"no time") << L" (avg. rate: " << Utils::nbytes_to_string(average_rate) << L"/s).\n" << std::endl;

	//Done
	return EXIT_SUCCESS;
}

static int retrieve_url(AbstractClient *const client, const std::wstring &url_string, const http_verb_t &http_verb, const URL &url, const std::wstring &post_data, const std::wstring &referrer, const std::wstring &outFileName, const bool &set_ftime, const bool &update_mode, const bool &alert, const bool &keep_failed)
{
	//Initialize the post data string
	const std::string post_data_encoded = post_data.empty() ? std::string() : ((post_data.compare(L"-") != 0) ? URL::urlEncode(Utils::wide_str_to_utf8(post_data)) : URL::urlEncode(stdin_get_line()));

	//Detect filestamp of existing file
	const uint64_t timestamp_existing = update_mode ? Utils::get_file_time(outFileName) : AbstractClient::TIME_UNKNOWN;
	if(update_mode && (timestamp_existing == AbstractClient::TIME_UNKNOWN))
	{
		std::wcerr << L"WARNING: Local file does not exist yet, going to download unconditionally!\n" << std::endl;
	}

	//Create the HTTPS connection/request
	std::wcerr << L"Connecting to " << url.getHostName() << L':' << url.getPortNo() << L", please wait..." << std::endl;
	std::unique_ptr<ConnectorThread> connector_thread (new ConnectorThread(client, http_verb, url, post_data_encoded, referrer, timestamp_existing));
	if(!connector_thread->start())
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << L"ERROR: Failed to start the file connector thread!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Wait for connection
	while(!connector_thread->join(Zero::g_sigUserAbort))
	{
		//Check for user abort
		if(ABORTED_BY_USER)
		{
			std::wcerr << L"--> Aborting!\n"<< std::endl;
			connector_thread->stop(1250, true);
			std::wcerr << L"SIGINT: Operation aborted by the user !!!\n" << std::endl;
			return EXIT_FAILURE;
		}
	}

	//Add extra space
	std::wcerr << std::endl;

	//Check thread result
	const uint32_t thread_result = connector_thread->get_result();
	if(thread_result != ConnectorThread::CONNECTION_COMPLETE)
	{
		const std::wstring error_text = connector_thread->get_error_text();
		switch(thread_result)
		{
		case ConnectorThread::CONNECTION_ERR_INET:
			if(!error_text.empty())
			{
				std::wcerr << error_text << L'\n' << std::endl;
			}
			std::wcerr << "ERROR: Connection could not be established!\n" << std::endl;
			break;
		case ConnectorThread::CONNECTION_ERR_ABRT:
			std::wcerr << L"ERROR: The operation has been aborted !!!\n" << std::endl;
			break;
		default:
			std::wcerr << L"ERROR: The operation failed for an unknwon reason!\n" << std::endl;
			break;
		}
		TRIGGER_SYSTEM_SOUND(alert, false);
		return EXIT_FAILURE;
	}

	//Check for user abort
	if(ABORTED_BY_USER)
	{
		std::wcerr << L"SIGINT: Operation aborted by the user !!!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Initialize local variables
	bool success;
	uint32_t status_code;
	std::wstring content_type, content_encd;
	uint64_t file_size, timestamp;

	//Query result information
	if(!client->result(success, status_code, file_size, timestamp, content_type, content_encd))
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		const std::wstring error_text = client->get_error_text();
		if(!error_text.empty())
		{
			std::wcerr << error_text << L'\n' << std::endl;
		}
		std::wcerr << "ERROR: Failed to query the response status!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Skip download this time?
	if(update_mode && (status_code == 304))
	{
		TRIGGER_SYSTEM_SOUND(alert, true);
		std::wcerr << L"SKIPPED: Server currently does *not* provide a newer version of the file." << std::endl;
		std::wcerr << L"         Version created at '" << Utils::timestamp_to_str(timestamp_existing) << L"' was retained.\n" << std::endl;
		return EXIT_SUCCESS;
	}

	//Print some status information
	std::wcerr << L"HTTP response successfully received from server:\n";
	print_response_info(status_code, file_size, timestamp, content_type, content_encd);

	//Request successful?
	if(!success)
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << "ERROR: The server failed to handle this request! [Status " << status_code << "]\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Check for user abort
	if(ABORTED_BY_USER)
	{
		std::wcerr << L"SIGINT: Operation aborted by the user !!!\n" << std::endl;
		return EXIT_FAILURE;
	}

	return transfer_file(client, url_string, file_size, (set_ftime ? timestamp : 0), outFileName, alert, keep_failed);
}

//=============================================================================
// MAIN
//=============================================================================

int inetget_main(const int argc, const wchar_t *const argv[])
{
	//Print application info
	print_logo();

	//Initialize parameters
	Params params;

	//Load configuration file, if it exists
	const std::wstring config_file = Utils::exe_path(L".cfg");
	if(Utils::file_exists(config_file))
	{
		if(!params.load_conf_file(config_file))
		{
			std::wcerr << "Invalid configuration file, refer to the documentation for details!\n" << std::endl;
			return EXIT_FAILURE;
		}
	}

	//Parse command-line parameters
	if(!params.parse_cli_args(argc, argv))
	{
		std::wcerr << "Invalid command-line arguments, type \"INetGet.exe --help\" for details!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Show help screen, if it was requested
	if(params.getShowHelp())
	{
		print_help_screen();
		return EXIT_SUCCESS;
	}

	//Parse the specified source URL
	const std::wstring source = (params.getSource().compare(L"-") == 0) ? Utils::utf8_to_wide_str(stdin_get_line()) : params.getSource();
	URL url(source);
	if(!url.isComplete())
	{
		std::wcerr << "The specified URL is incomplete or unsupported:\n" << source << L'\n' << std::endl;
		return EXIT_FAILURE;
	}

	//Print request URL
	const std::wstring url_string = url.toString();
	std::wcerr << L"Request address:\n" << url.toString() << L'\n' << std::endl;
	Utils::set_console_title(std::wstring(L"INetGet - ").append(url_string));

	//Create the HTTP(S) client
	std::unique_ptr<AbstractClient> client;
	StatusListener listener;
	if(!create_client(client, listener, url.getScheme(), params))
	{
		std::wcerr << "Specified protocol is unsupported! Only HTTP(S) and FTP are allowed.\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Retrieve the URL
	return retrieve_url(client.get(), url_string, params.getHttpVerb(), url, params.getPostData(), params.getReferrer(), params.getOutput(), params.getSetTimestamp(), params.getUpdateMode(), params.getEnableAlert(), params.getKeepFailed());
}
