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

#include "Client_HTTP.h"

//Internal
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinInet.h>

//CRT
#include <stdint.h>
#include <iostream>
#include <stdexcept>

//Helper macro
static const wchar_t *CSTR(const std::wstring &str) { return str.empty() ? NULL : str.c_str(); }

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

HttpClient::HttpClient(const bool &disableProxy, const std::wstring &userAgentStr, const bool &verbose)
:
	m_hConnection(NULL),
	m_hRequest(NULL),
	AbstractClient(disableProxy, userAgentStr, verbose)
{
}

HttpClient::~HttpClient(void)
{
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool HttpClient::open(const http_verb_t &verb, const bool &secure, const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password, const std::wstring &path)
{
	if(!wininet_init())
	{
		return false; /*WinInet failed to initialize*/
	}

	//Close the existing connection, just to be sure
	if(!close())
	{
		std::wcerr << L"ERROR: Failed to close the existing connection!\n" << std::endl;
		return false;
	}

	//Print info
	std::wcerr << L"Creating " << (secure ? L"HTTPS" : L"HTTP") << " connection to " << hostName << L':' << portNo << ", please wait..." << std::endl;

	//Create connection
	if(!connect(hostName, portNo, userName, password))
	{
		return false; /*the connection could not be created*/
	}

	//Create HTTP request and send!
	if(!create_request(secure, verb, path))
	{
		return false; /*the request could not be created or sent*/
	}

	//Sucess
	std::wcerr << L"Connected.\n" << std::endl;
	return true;
}

bool HttpClient::close(void)
{
	bool success = true;

	//Close the request, if it currently exists
	if(!close_handle(m_hRequest))
	{
		success = false;
	}

	//Close connection, if it is currently open
	if(!close_handle(m_hConnection))
	{
		success = false;
	}

	return success;
}

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

bool HttpClient::connect(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password)
{
	//Try to open the new connection
	m_hConnection = InternetConnect(m_hInternet, CSTR(hostName), portNo, CSTR(userName), CSTR(password), INTERNET_SERVICE_HTTP, 0, reinterpret_cast<intptr_t>(this));
	if(m_hConnection == NULL)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "Failed!\n\nInternetConnect() function has failed:\n" << error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	//Install the callback handler (only in verbose mode)
	if(m_verbose && (m_hConnection != NULL))
	{
		if(InternetSetStatusCallback(m_hConnection, (INTERNET_STATUS_CALLBACK)(&callback_handler)) == INTERNET_INVALID_STATUS_CALLBACK)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "Failed!\n\nInternetSetStatusCallback() function has failed:\n" << error_string(error_code) << L'\n' << std::endl;
			return false;
		}
	}

	return true;
}

bool HttpClient::create_request(const bool &secure, const http_verb_t &verb, const std::wstring &path)
{
	//Setup request flags
	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	if(secure)
	{
		flags |= INTERNET_FLAG_SECURE;
	}

	//Try to create the HTTP request
	m_hRequest = HttpOpenRequest(m_hConnection, get_verb(verb), CSTR(path), L"HTTP/1.1", NULL, NULL, flags, intptr_t(this));
	if(m_hRequest == NULL)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "Failed!\n\nHttpOpenRequest() function has failed:\n" << error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	//Try to actually send the HTTP request
	BOOL success = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
	if(success != TRUE)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "Failed!\n\nConnection to server could not be established:\n" << error_string(error_code) << L'\n' << std::endl;
		return false;
	}
		
	return true;
}

//=============================================================================
// QUERY STATUS
//=============================================================================

bool HttpClient::result(bool &success, uint32_t &status_code, uint32_t &file_size, std::wstring &content_type)
{
	if(m_hRequest == NULL)
	{
		std::wcerr << "INTERNAL ERROR: There currently is no active request!" << std::endl;
		return false; /*request not created yet*/
	}

	if(!get_header_int(m_hRequest, HTTP_QUERY_STATUS_CODE, status_code))
	{
		status_code = 0;
	}

	if(!get_header_int(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, file_size))
	{
		file_size = SIZE_UNKNOWN;
	}

	if(!get_header_str(m_hRequest, HTTP_QUERY_CONTENT_TYPE, content_type))
	{
		content_type.clear();
	}

	success = ((status_code >= 200) && (status_code < 300));
	return (status_code > 0);
}

//=============================================================================
// UTILITIES
//=============================================================================

#define CHECK_HTTP_VERB(X) do \
{ \
	if((X) == verb) \
	{ \
		static const wchar_t *const name = L#X; \
		return &name[5]; \
	} \
} \
while(0)

const wchar_t *HttpClient::get_verb(const http_verb_t &verb)
{
	CHECK_HTTP_VERB(HTTP_GET);
	CHECK_HTTP_VERB(HTTP_POST);
	CHECK_HTTP_VERB(HTTP_PUT);
	CHECK_HTTP_VERB(HTTP_DELETE);
	CHECK_HTTP_VERB(HTTP_HEAD);
	CHECK_HTTP_VERB(HTTP_OPTIONS);
	CHECK_HTTP_VERB(HTTP_TRACE);

	throw new std::runtime_error("Invalid verb specified!");
}

bool HttpClient::get_header_int(void *const request, const uint32_t type, uint32_t &value)
{
	DWORD result;
	DWORD resultSize = sizeof(DWORD);

	if(HttpQueryInfo(request, type | HTTP_QUERY_FLAG_NUMBER, &result, &resultSize, 0) == TRUE)
	{
		value = result;
		return true;
	}

	const DWORD error_code = GetLastError();
	if(error_code != ERROR_HTTP_HEADER_NOT_FOUND)
	{
		std::wcerr << "HttpQueryInfo() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
	}

	return false;
}

bool HttpClient::get_header_str(void *const request, const uint32_t type, std::wstring &value)
{
	static const size_t BUFF_SIZE = 2048;
	wchar_t result[BUFF_SIZE];
	DWORD resultSize = BUFF_SIZE * sizeof(wchar_t);

	if(HttpQueryInfo(request, type, &result, &resultSize, 0) == TRUE)
	{
		value = std::wstring(result, resultSize / sizeof(wchar_t));
		return true;
	}

	const DWORD error_code = GetLastError();
	if(error_code != ERROR_HTTP_HEADER_NOT_FOUND)
	{
		std::wcerr << "HttpQueryInfo() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
	}

	return false;
}
