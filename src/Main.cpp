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

//Win32
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

#if (defined(NDEBUG) && defined(_DEBUG)) || ((!defined(NDEBUG)) && (!defined(_DEBUG)))
#error Inconsistent DEBUG flags!
#endif

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

static std::wstring status_to_string(const uint32_t &status_code)
{
	std::wostringstream str;
	str << status_code << " [";
	bool found = false;
	for(size_t i = 0; STATUS_CODES[i].info; i++)
	{
		if(STATUS_CODES[i].code == status_code)
		{
			found = true;
			str << STATUS_CODES[i].info << "]\n";
			break;
		}
	}
	if(!found)
	{
		str << "Unknown]\n";
	}
	return str.str();
}

static void print_response_info(const uint32_t &status_code, const uint32_t &file_size,	const std::wstring &content_type, const std::wstring &content_encd)
{
	static const wchar_t *const UNSPECIFIED = L"<N/A>";

	std::wcerr << L"--> Status code: "      << status_to_string(status_code);
	std::wcerr << L"--> Content type: "     << (content_type.empty() ? UNSPECIFIED : content_type) << L'\n';
	std::wcerr << L"--> Content encoding: " << (content_encd.empty() ? UNSPECIFIED : content_encd) << L'\n';
	std::wcerr << L"--> Length (bytes): "   << ((file_size == AbstractClient::SIZE_UNKNOWN) ? UNSPECIFIED : std::to_wstring(file_size)) << L'\n';
	std::wcerr << std::endl;
}

//=============================================================================
// PROCESS
//=============================================================================

static int retrieve_url(AbstractClient *const client, const URL &url)
{
	//Create the HTTPS connection/request
	if(!client->open(HTTP_GET, IS_HTTPS(url), url.getHostName(), url.getPort(), url.getUserName(), url.getPassword(), url.getUrlPath()))
	{
		std::wcerr << "ERROR: The request could not be sent!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Query result information
	uint32_t status_code, file_size;
	bool success;
	std::wstring content_type, content_encd;
	if(!client->result(success, status_code, file_size, content_type, content_encd))
	{
		std::wcerr << "ERROR: Failed to query the response status!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Print some status information
	std::wcerr << L"HTTP response has been received from server:\n";
	print_response_info(status_code, file_size, content_type, content_encd);

	//Request successful?
	if(!success)
	{
		std::wcerr << "ERROR: The server failed to handle this request! [Status " << status_code << "]\n" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
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
	std::wcerr << L"Request URL:\n" << params.getSource() << L'\n' << std::endl;

	//Create the HTTP(S) client
	std::unique_ptr<AbstractClient> client;
	if(!create_client(client, url.getScheme(), params.getDisableProxy(), params.getUserAgent(), params.getVerboseMode()))
	{
		std::wcerr << "Specified protocol is unsupported! Only HTTP and HTTPS are currently allowed.\n" << std::endl;
		return EXIT_FAILURE;
	}

	//Retrieve the URL
	return retrieve_url(client.get(), url);
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
