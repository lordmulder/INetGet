///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version, but always including the *additional*
// restrictions defined in the "License.txt" file.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// http://www.gnu.org/licenses/gpl-2.0.txt
///////////////////////////////////////////////////////////////////////////////

//Internal
#include "Version.h"
#include "URL.h"
#include "Utils.h"
#include "Params.h"
#include "Client_FTP.h"
#include "Client_HTTP.h"
#include "Sink_File.h"
#include "Sink_StdOut.h"
#include "Sink_Null.h"
#include "Timer.h"
#include "Average.h"

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

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

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
		str << std::setw(0) << VERSION_MAJOR << L'.' << std::setfill(L'0') << std::setw(2) << VERSION_MINOR << L'_' << std::setw(0) << VERSION_PATCH;
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
	std::wcerr << L"\nINetGet v" << build_version_string() << " - Lightweight command-line front-end to WinINet\n"
		<< L"Copyright (c) " << &BUILD_DATE[7] << L" LoRd_MuldeR <mulder2@gmx.de>. Some rights reserved.\n"
		<< L"Built on " << BUILD_DATE << " at " << BUILD_TIME << ", " << BUILD_COMP << ", Win-" << BUILD_ARCH << ", " << BUILD_CONF << '\n' << std::endl;
	std::wcout.flags(stateBackup);
}

static void print_help_screen(void)
{
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr
		<< L"Check http://muldersoft.com/ or https://github.com/lordmulder/ for updates!\n"
		<< L'\n'
		<< L"Usage:\n"
		<< L"  INetGet.exe [options] <source_addr> <output_file>\n"
		<< L'\n'
		<< L"Required:\n"
		<< L"  <source_addr> : Specifies the source internet address (URL)\n"
		<< L"  <output_file> : Specifies the output file, you can specify \"-\" for STDOUT\n"
		<< L'\n'
		<< L"Optional:\n"
		<< L"  --verb=<verb> : Specify the HTTP method (verb) to be used, default is GET\n"
		<< L"  --data=<data> : Append data to request, in 'x-www-form-urlencoded' format\n"
		<< L"  --no-proxy    : Don't use proxy server for address resolution\n"
		<< L"  --agent=<str> : Overwrite the default 'user agent' string used by INetGet\n"
		<< L"  --no-redir    : Disable automatic redirection, enabled by default\n"
		<< L"  --insecure    : Don't fail, if server certificate is invalid (HTTPS only)\n"
		<< L"  --notify      : Trigger a system sound when the download completed/failed\n"
		<< L"  --help        : Show this help screen\n"
		<< L"  --verbose     : Enable detailed diagnostic output (for debugging)\n"
		<< L'\n'
		<< L"Examples:\n"
		<< L"  INetGet.exe http://www.warr.org/buckethead.html output.html\n"
		<< L"  INetGet.exe --verb=POST --data=\"foo=bar\" http://localhost/form.php result\n"
		<< std::endl;
	std::wcout.flags(stateBackup);
}

static bool create_client(std::unique_ptr<AbstractClient> &client, const int16_t scheme_id, const bool &disableProxy, const std::wstring userAgentStr, const bool &verbose)
{
	switch(scheme_id)
	{
	case INTERNET_SCHEME_FTP:
		client.reset(new FtpClient(disableProxy, userAgentStr, verbose));
		break;
	case INTERNET_SCHEME_HTTP:
	case INTERNET_SCHEME_HTTPS:
		client.reset(new HttpClient(disableProxy, userAgentStr, verbose));
		break;
	default:
		client.reset();
		break;
	}

	return !!client;
}

static bool create_sink(std::unique_ptr<AbstractSink> &sink, const std::wstring fileName)
{
	if(_wcsicmp(fileName.c_str(), L"-") == 0)
	{
		sink.reset(new StdOutSink());
	}
	else if(_wcsicmp(fileName.c_str(), L"NULL") == 0)
	{
		sink.reset(new NullSink());
	}
	else
	{
		sink.reset(new FileSink(fileName));
	}

	return sink ? sink->open() : false;
}

static void print_response_info(const uint32_t &status_code, const uint64_t &file_size,	const std::wstring &content_type, const std::wstring &content_encd)
{
	static const wchar_t *const UNSPECIFIED = L"<N/A>";

	std::wcerr << L"--> Status code: "      << status_code << L" [" << status_to_string(status_code) << "]\n";
	std::wcerr << L"--> Content type: "     << (content_type.empty() ? UNSPECIFIED : content_type) << L'\n';
	std::wcerr << L"--> Content encoding: " << (content_encd.empty() ? UNSPECIFIED : content_encd) << L'\n';
	std::wcerr << L"--> Length (bytes): "   << ((file_size == AbstractClient::SIZE_UNKNOWN) ? UNSPECIFIED : std::to_wstring(file_size)) << L'\n';
	std::wcerr << std::endl;
}

static inline void print_progress(const uint64_t &total_bytes, const uint64_t &file_size, Timer &timer_update, const double &current_rate, uint8_t &index, const bool &force = false)
{
	static const wchar_t SPINNER[4] = { L'-', L'\\', L'/', L'-' };

	if(force || (timer_update.query() > 0.2))
	{
		const std::ios::fmtflags stateBackup(std::wcout.flags());
		std::wcerr << std::setprecision(1) << std::fixed << std::setw(0) << "\r[" << SPINNER[(index++) & 3] << "] ";

		if(file_size != AbstractClient::SIZE_UNKNOWN)
		{
			const double percent = (file_size > 0.0) ? (100.0 * std::min(1.0, double(total_bytes) / double(file_size))) : 100.0;
			if(!std::isnan(current_rate))
			{
				if(current_rate > 0.0)
				{
					const double time_left = (total_bytes < file_size) ? (double(file_size - total_bytes) / current_rate) : 0.0;
					if(time_left > 3)
					{
						std::wcerr << percent << "% of " << nbytes_to_string(double(file_size)) << " received, " << nbytes_to_string(current_rate) << "/s, " << second_to_string(time_left) << " remaining...";
					}
					else
					{
						std::wcerr << percent << "% of " << nbytes_to_string(double(file_size)) << " received, " << nbytes_to_string(current_rate) << "/s, almost finished...";
					}
				}
				else
				{
					std::wcerr << percent << "% of " << nbytes_to_string(double(file_size)) << " received, " << nbytes_to_string(current_rate) << "/s, please stand by...";
				}
			}
			else
			{
				std::wcerr << percent << "% of " << nbytes_to_string(double(file_size)) << " received, please stand by...";
			}
		}
		else
		{
			if(!std::isnan(current_rate))
			{
				std::wcerr << nbytes_to_string(double(total_bytes)) << " received, " << nbytes_to_string(current_rate) << "/s, please stand by...";
			}
			else
			{
				std::wcerr << nbytes_to_string(double(total_bytes)) << " received, please stand by...";
			}
		}

		std::wcerr << "    " << std::flush;
		std::wcout.flags(stateBackup);
		timer_update.reset();
	}
}

//=============================================================================
// PROCESS
//=============================================================================

static int transfer_file(AbstractClient *const client, const uint64_t &file_size, const std::wstring &outFileName, const bool &alert)
{
	//Open output file
	std::unique_ptr<AbstractSink> sink;
	if(!create_sink(sink, outFileName))
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << "ERROR: Failed to open the sink, unable to download file!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Allocate buffer
	static const size_t BUFF_SIZE = 16384;
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[BUFF_SIZE]);

	//Initialize local variables
	Timer timer_start, timer_transfer, timer_update;
	uint64_t total_bytes = 0ui64, transferred_bytes = 0ui64;
	uint8_t index = 0;
	Average rate_estimate(125);
	bool eof_flag = false;
	double current_rate = std::numeric_limits<double>::quiet_NaN();;

	//Print progress
	std::wcerr << L"Download in progress:" << std::endl;
	print_progress(total_bytes, file_size, timer_update, current_rate, index, true);

	//Download file
	while(!eof_flag)
	{
		size_t bytes_read = 0;

		CHECK_USER_ABORT();
		if(!client->read_data(buffer.get(), BUFF_SIZE, bytes_read, eof_flag))
		{
			TRIGGER_SYSTEM_SOUND(alert, false);
			std::wcerr << "ERROR: Failed to receive incoming data, download has been aborted!\n" << std::endl;
			return EXIT_FAILURE;
		}

		if(bytes_read > 0)
		{
			CHECK_USER_ABORT();
			transferred_bytes += bytes_read;
			total_bytes += bytes_read;

			const double interval = timer_transfer.query();
			if(interval >= 0.5)
			{
				current_rate = rate_estimate.update(double(transferred_bytes) / interval);
				timer_transfer.reset();
				transferred_bytes = 0ui64;
			}

			if(!sink->write(buffer.get(), bytes_read))
			{
				TRIGGER_SYSTEM_SOUND(alert, false);
				std::wcerr << "ERROR: Failed to write data to sink, download has been aborted!\n" << std::endl;
				return EXIT_FAILURE;
			}
		}

		CHECK_USER_ABORT();
		print_progress(total_bytes, file_size, timer_update, current_rate, index);
	}

	print_progress(total_bytes, file_size, timer_update, current_rate, index, true);
	const double total_time = timer_start.query(), average_rate = total_bytes / total_time;

	std::wcerr << "\b\b\bdone\n\nFlushing output buffers... " << std::flush;
	sink->close();

	TRIGGER_SYSTEM_SOUND(alert, true);
	std::wcerr << "done\n\nDownload completed in " << ((total_time >= 1.0) ? second_to_string(total_time) : L"no time") << " (avg. rate: " << nbytes_to_string(average_rate) << "/s).\n" << std::endl;
	return EXIT_SUCCESS;
}

static int retrieve_url(AbstractClient *const client, const http_verb_t &http_verb, const URL &url, const std::wstring &post_data, const std::wstring &outFileName, const bool &no_redir, const bool &insecure, const bool &alert)
{
	//Initialize the post data string
	const std::string post_data_utf8 = post_data.empty() ? std::string() : ((post_data.compare(L"-") != 0) ? wide_str_to_utf8(post_data) : stdin_get_line());

	//Create the HTTPS connection/request
	if(!client->open(http_verb, url, post_data_utf8, no_redir, insecure))
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << "ERROR: The request could not be sent!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Initialize local variables
	bool success;
	uint32_t status_code;
	uint64_t file_size;
	std::wstring content_type, content_encd;

	//Query result information
	CHECK_USER_ABORT();
	if(!client->result(success, status_code, file_size, content_type, content_encd))
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << "ERROR: Failed to query the response status!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Print some status information
	std::wcerr << L"HTTP response successfully received from server:\n";
	print_response_info(status_code, file_size, content_type, content_encd);

	//Request successful?
	if(!success)
	{
		TRIGGER_SYSTEM_SOUND(alert, false);
		std::wcerr << "ERROR: The server failed to handle this request! [Status " << status_code << "]\n" << std::endl;
		return EXIT_FAILURE;
	}

	CHECK_USER_ABORT();
	return transfer_file(client, file_size, outFileName, alert);
}

//=============================================================================
// MAIN
//=============================================================================

int inetget_main(const int argc, const wchar_t *const argv[])
{
	//Print application info
	print_logo();
	
	//Parse command-line parameters
	Params params;
	if(!params.initialize(argc, argv))
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
	const std::wstring source = (params.getSource().compare(L"-") == 0) ? utf8_to_wide_str(stdin_get_line()) : params.getSource();
	URL url(source);
	if(!url.isComplete())
	{
		std::wcerr << "The specified URL is incomplete or unsupported:\n" << source << L'\n' << std::endl;
		return EXIT_FAILURE;
	}

	//Print request URL
	std::wcerr << L"Request address:\n" << source << L'\n' << std::endl;

	//Create the HTTP(S) client
	std::unique_ptr<AbstractClient> client;
	if(!create_client(client, url.getScheme(), params.getDisableProxy(), params.getUserAgent(), params.getVerboseMode()))
	{
		std::wcerr << "Specified protocol is unsupported! Only HTTP(S) and FTP are allowed.\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Retrieve the URL
	return retrieve_url(client.get(), params.getHttpVerb(), url, params.getPostData(), params.getOutput(), params.getDisableRedir(), params.getInsecure(), params.getEnableAlert());
}
