///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinInet API
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
#include "Client_HTTP.h"
#include "Sink_File.h"
#include "Sink_StdOut.h"
#include "Sink_Null.h"
#include "Average.h"

//Win32
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinInet.h>

//CRT
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <io.h>
#include <fcntl.h>
#include <memory>
#include <sstream>
#include <algorithm>

#if (defined(NDEBUG) && defined(_DEBUG)) || ((!defined(NDEBUG)) && (!defined(_DEBUG)))
#error Inconsistent DEBUG flags!
#endif

#define UPDATE(OLD,NEW,ALPHA) (((OLD) * (1.0 - (ALPHA))) + ((NEW) * (ALPHA)))
#define IS_HTTPS(URL) ((URL).getScheme() == INTERNET_SCHEME_HTTPS)

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

static std::wstring build_version_string(void)
{
	std::wostringstream str;
	if(VER_INETGET_PATCH > 0)
	{
		str << std::setw(0) << VER_INETGET_MAJOR << L'.' << std::setfill(L'0') << std::setw(2) << VER_INETGET_MINOR << L'_' << std::setw(0) << VER_INETGET_PATCH;
	}
	else
	{
		str << std::setw(0) << VER_INETGET_MAJOR << L'.' << std::setfill(L'0') << std::setw(2) << VER_INETGET_MINOR;
	}
	return str.str();
}

static void print_logo(void)
{
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr << L"\nINetGet v" << build_version_string() << " - Lightweight command-line front-end to WinInet\n"
		<< L"Copyright (c) " << &BUILD_DATE[7] << L" LoRd_MuldeR <mulder2@gmx.de>. Some rights reserved.\n"
		<< L"Built on " << BUILD_DATE << " at " << BUILD_TIME << " with " << BUILD_COMP << " (" << BUILD_ARCH << ")\n" << std::endl;
	std::wcout.flags(stateBackup);
}

static void print_help_screen(void)
{
	const std::ios::fmtflags stateBackup(std::wcout.flags());
	std::wcerr << L"Usage:\n"
		<< L"  INetGet.exe [options] <source_addr> <output_file>\n"
		<< L'\n'
		<< L"Required:\n"
		<< L"  <source_addr> : Specifies the source internet address (URL)\n"
		<< L"  <output_file> : Specifies the output file, you can specify \"-\" for STDOUT\n"
		<< L'\n'
		<< L"Optional:\n"
		<< L"  --no-proxy    : Don't use proxy server for address resolution\n"
		<< L"  --agent=<str> : Overwrite the default 'user agent' string used by INetGet\n"
		<< L"  --verbose     : Enable detailed diagnostic output (for debugging)\n"
		<< L"  --help        : Show this help screen\n"
		<< std::endl;
	std::wcout.flags(stateBackup);
}

static bool create_client(std::unique_ptr<AbstractClient> &client, const int16_t scheme_id, const bool &disableProxy, const std::wstring userAgentStr, const bool &verbose)
{
	switch(scheme_id)
	{
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

static inline void print_progress(const uint64_t &total_bytes, const uint64_t &file_size, uint64_t &last_update, uint64_t &last_bytes, Average &rate_estimate, uint8_t &index, const bool &force = false)
{
	static const wchar_t SPINNER[4] = { L'-', L'\\', L'/', L'-' };
	static const uint64_t UPDATE_INTERVAL = TICKS_PER_SECCOND / 4;

	const uint64_t current_time = get_system_time();
	if(force || ((current_time - last_update) > UPDATE_INTERVAL))
	{
		double current_rate = -1.0;
		if((last_update > 0) && (total_bytes > 0))
		{
			current_rate = rate_estimate.update((double(total_bytes - last_bytes) / double(current_time - last_update)) * double(TICKS_PER_SECCOND));
		}

		last_update = current_time;
		last_bytes = total_bytes;

		const std::ios::fmtflags stateBackup(std::wcout.flags());
		std::wcerr << std::setprecision(1) << std::fixed << std::setw(0);

		if((file_size > 0) && (file_size != AbstractClient::SIZE_UNKNOWN))
		{
			const double percent = 100.0 * std::min(1.0, double(total_bytes) / double(file_size));
			if(current_rate >= 0.0)
			{
				const double time_left = (total_bytes < file_size) ? (double(file_size - total_bytes) / current_rate) : 0.0;
				if(time_left > 3)
				{
					std::wcerr << "\r[" << SPINNER[(index++) & 3] << "] " << percent << "% of " << bytes_to_string(double(file_size)) << " received, " << bytes_to_string(current_rate) << "/s, " << ticks_to_string(time_left) << " remaining...   " << std::flush;
				}
				else
				{
					std::wcerr << "\r[" << SPINNER[(index++) & 3] << "] " << percent << "% of " << bytes_to_string(double(file_size)) << " received, " << bytes_to_string(current_rate) << "/s, almost finished...   " << std::flush;
				}
			}
			else
			{
				std::wcerr << "\r[" << SPINNER[(index++) & 3] << "] " << percent << "% of " << bytes_to_string(double(file_size)) << " received, please stand by...   " << std::flush;
			}
		}
		else
		{
			if(current_rate >= 0.0)
			{
				std::wcerr << "\r[" << SPINNER[(index++) & 3] << "] " << bytes_to_string(double(file_size)) << " received, " << bytes_to_string(current_rate) << "/s, please stand by...   " << std::flush;
			}
			else
			{
				std::wcerr << "\r[" << SPINNER[(index++) & 3] << "] " << bytes_to_string(double(file_size)) << " received, please stand by...   " << std::flush;
			}
		}

		std::wcout.flags(stateBackup);
	}
}

//=============================================================================
// PROCESS
//=============================================================================

static int transfer_file(AbstractClient *const client, const uint64_t &file_size, const std::wstring &outFileName)
{
	//Open output file
	std::unique_ptr<AbstractSink> sink;
	if(!create_sink(sink, outFileName))
	{
		std::wcerr << "ERROR: Failed to open the sink, unable to download file!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Allocate buffer
	static const size_t BUFF_SIZE = 16384;
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[BUFF_SIZE]);

	//Initialize local variables
	const uint64_t time_start = get_system_time();
	uint64_t total_bytes = 0, last_update = 0, last_bytes = 0;
	uint8_t index = 0;
	Average rate_estimate(255);
	bool eof_flag = false;

	//Print progress
	std::wcerr << L"Download in progress:" << std::endl;
	print_progress(total_bytes, file_size, last_update, last_bytes, rate_estimate, index, true);

	//Download file
	while(!eof_flag)
	{
		size_t bytes_read = 0;
		if(!client->read_data(buffer.get(), BUFF_SIZE, bytes_read, eof_flag))
		{
			std::wcerr << "ERROR: Failed to receive incoming data, download has failed!\n" << std::endl;
			return EXIT_FAILURE;
		}

		if(bytes_read > 0)
		{
			total_bytes += bytes_read;
			if(!sink->write(buffer.get(), bytes_read))
			{
				std::wcerr << "ERROR: Failed to write data to sink, download has failed!\n" << std::endl;
				return EXIT_FAILURE;
			}
		}

		print_progress(total_bytes, file_size, last_update, last_bytes, rate_estimate, index);
	}

	print_progress(total_bytes, file_size, last_update, last_bytes, rate_estimate, index, true);
	std::wcerr << "\b\bdone\n\nDownload completed successfully in " << double(get_system_time() - time_start) / TICKS_PER_SECCOND << " seconds.\n" << std::endl;
	return EXIT_SUCCESS;
}

static int retrieve_url(AbstractClient *const client, const URL &url, const std::wstring &outFileName)
{
	//Create the HTTPS connection/request
	if(!client->open(HTTP_GET, IS_HTTPS(url), url.getHostName(), url.getPort(), url.getUserName(), url.getPassword(), url.getUrlPath()))
	{
		std::wcerr << "ERROR: The request could not be sent!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Query result information
	bool success;
	uint32_t status_code;
	uint64_t file_size;
	std::wstring content_type, content_encd;
	if(!client->result(success, status_code, file_size, content_type, content_encd))
	{
		std::wcerr << "ERROR: Failed to query the response status!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Print some status information
	std::wcerr << L"HTTP response successfully received from server:\n";
	print_response_info(status_code, file_size, content_type, content_encd);

	//Request successful?
	if(!success)
	{
		std::wcerr << "ERROR: The server failed to handle this request! [Status " << status_code << "]\n" << std::endl;
		return EXIT_FAILURE;
	}

	return transfer_file(client, file_size, outFileName);
}

//=============================================================================
// MAIN
//=============================================================================

static int inetget_main(const int argc, const wchar_t *const argv[])
{
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stderr), _O_U8TEXT);

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
	URL url(params.getSource());
	if(!url.isComplete())
	{
		std::wcerr << "The specified URL is incomplete or unsupported:\n" << params.getSource() << L'\n' << std::endl;
		return EXIT_FAILURE;
	}

	//Print request URL
	std::wcerr << L"Request address:\n" << params.getSource() << L'\n' << std::endl;

	//Create the HTTP(S) client
	std::unique_ptr<AbstractClient> client;
	if(!create_client(client, url.getScheme(), params.getDisableProxy(), params.getUserAgent(), params.getVerboseMode()))
	{
		std::wcerr << "Specified protocol is unsupported! Only HTTP and HTTPS are currently allowed.\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Retrieve the URL
	return retrieve_url(client.get(), url, params.getOutput());
}

//=============================================================================
// ERROR HANDLING
//=============================================================================

static void my_invalid_param_handler(const wchar_t* exp, const wchar_t* fun, const wchar_t* fil, unsigned int, uintptr_t)
{
	std::wcerr << "\n\nGURU MEDITATION: Invalid parameter handler invoked, application will exit!\n" << std::endl;
	_exit(EXIT_FAILURE);
}

static LONG WINAPI my_exception_handler(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	std::wcerr << "\n\nGURU MEDITATION: Unhandeled exception handler invoked, application will exit!\n" << std::endl;
	_exit(EXIT_FAILURE);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void setup_error_handlers(void)
{
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	SetUnhandledExceptionFilter(my_exception_handler);
	_set_invalid_parameter_handler(my_invalid_param_handler);
	SetDllDirectoryW(L""); /*don'tload DLL from "current" directory*/
}

//=============================================================================
// ENTRY POINT
//=============================================================================

static int wmain_ex(const int argc, const wchar_t *const argv[])
{
	int ret = -1;
	try
	{
		ret = inetget_main(argc, argv);
	}
	catch(std::exception &err)
	{
		std::wcerr << "\n\nUNHANDELED EXCEPTION: " << err.what() << '\n' << std::endl;
		_exit(EXIT_FAILURE);
	}
	catch(...)
	{
		std::wcerr << "\n\nUNHANDELED EXCEPTION: Unknown C++ exception error!\n" << std::endl;
		_exit(EXIT_FAILURE);
	}
	return ret;
}

int wmain(int argc, wchar_t* argv[])
{
	int ret = -1;
#ifdef NDEBUG
	__try
	{
		setup_error_handlers();
		ret = wmain_ex(argc, argv);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		std::wcerr << "\n\nGURU MEDITATION: Unhandeled exception error, application will exit!\n" << std::endl;
	}
#else
	ret =  inetget_main(argc, argv);
#endif
	return ret;
}
