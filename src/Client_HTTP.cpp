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

HttpClient::HttpClient(const bool &verbose)
:
	AbstractClient(verbose)
{
}

HttpClient::~HttpClient(void)
{
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool HttpClient::connection_init(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password)
{
	return AbstractClient::connection_init(INTERNET_SERVICE_HTTP, hostName, portNo, userName, password);
}

//=============================================================================
// REQUEST HANDLING
//=============================================================================

bool HttpClient::request_init(const std::wstring &verb, const std::wstring &path, const bool &secure)
{
	if(m_hConnection == NULL)
	{
		throw std::runtime_error("Connection not established yet!");
	}
	
	//Close existing request, just to be sure
	if(!request_exit())
	{
		std::wcerr << "ERROR: Failed to close the existing connection!\n" << std::endl;
		return false;
	}

	//Setup connection flags
	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	if(secure)
	{
		flags |= INTERNET_FLAG_SECURE;
	}

	//Try to create the HTTP request
	m_hRequest = HttpOpenRequest(m_hConnection, CSTR(verb), CSTR(path), L"HTTP/1.1", NULL, NULL, flags, intptr_t(this));
	if(m_hRequest == NULL)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "HttpOpenRequest() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
	}

	//Try to actually send the HTTP request
	if(m_hRequest != NULL)
	{
		std::wcerr << L"Creating " << (secure ? L"HTTPS" : L"HTTP") << " connection, please wait..." << std::endl;
		BOOL success = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
		if(success != TRUE)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "Failed!\n\nHttpSendRequest() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
			return false;
		}
		
		std::wcerr << "Successful.\n" << std::endl;
		return true;
	}

	return false;
}

//=============================================================================
// QUERY STATUS
//=============================================================================

static bool get_header_int(void *const request, const DWORD type, uint32_t &value)
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

static bool get_header_str(void *const request, const DWORD type, std::wstring &value)
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

bool HttpClient::query_result(bool &success, uint32_t &status_code, uint32_t &file_size, std::wstring &content_type)
{
	if(m_hRequest == NULL)
	{
		throw std::runtime_error("Request not created yet!");
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
