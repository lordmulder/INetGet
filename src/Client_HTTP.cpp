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
	m_current_status(UINT32_MAX),
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
	std::wcerr << L"Creating " << (secure ? L"HTTPS" : L"HTTP") << " connection to " << hostName << L':' << portNo << ", please wait:" << std::endl;

	//Reset status
	m_current_status = UINT32_MAX;

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
	std::wcerr << L"--> Response received.\n" << std::endl;
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

	CHECK_USER_ABORT();
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
		std::wcerr << "--> Failed!\n\nInternetConnect() function has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	//Install the callback handler (only in verbose mode)
	if(InternetSetStatusCallback(m_hConnection, (INTERNET_STATUS_CALLBACK)(&status_callback)) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "--> Failed!\n\nInternetSetStatusCallback() function has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	CHECK_USER_ABORT();
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
		std::wcerr << "--> Failed!\n\nHttpOpenRequest() function has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	//Try to actually send the HTTP request
	BOOL success = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
	if(success != TRUE)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "--> Failed!\n\nFailed to connect to the server:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}
	
	CHECK_USER_ABORT();
	return true;
}

//=============================================================================
// QUERY RESULT
//=============================================================================

bool HttpClient::result(bool &success, uint32_t &status_code, uint64_t &file_size, std::wstring &content_type, std::wstring &content_encd)
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

	uint32_t content_length = 0;
	if(!get_header_int(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, content_length))
	{
		content_length = 0;
	}

	if(!get_header_str(m_hRequest, HTTP_QUERY_CONTENT_TYPE, content_type))
	{
		content_type.clear();
	}

	if(!get_header_str(m_hRequest, HTTP_QUERY_CONTENT_ENCODING, content_encd))
	{
		content_encd.clear();
	}

	success = ((status_code >= 200) && (status_code < 300));
	file_size = ((content_length > 0) && (content_length < UINT32_MAX)) ? uint64_t(content_length) : SIZE_UNKNOWN;
	return (status_code > 0);
}

//=============================================================================
// READ PAYLOAD
//=============================================================================

bool HttpClient::read_data(uint8_t *out_buff, const size_t &buff_size, size_t &bytes_read, bool &eof_flag)
{
	if(m_hRequest == NULL)
	{
		std::wcerr << "\n\nINTERNAL ERROR: There currently is no active request!" << std::endl;
		return false; /*request not created yet*/
	}

	DWORD temp;
	if(!InternetReadFile(m_hRequest, out_buff, buff_size, &temp))
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "\b\b\bfailed!\n\nAn error occurred while receiving data from the server:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	eof_flag = ((bytes_read = temp) < 1);
	return true;
}

//=============================================================================
// STATUS HANDLER
//=============================================================================

void HttpClient::update_status(const uint32_t &status, const uintptr_t &info)
{
	CHECK_USER_ABORT();
	AbstractClient::update_status(status, info);
	if(m_current_status != status)
	{
		bool processed = false;
		switch(status)
		{
			case INTERNET_STATUS_DETECTING_PROXY:      processed = true; std::wcerr << "--> Detetcing proxy server..."                      << std::endl; break;
			case INTERNET_STATUS_RESOLVING_NAME:       processed = true; std::wcerr << "--> Resolving host name..."                         << std::endl; break;
			case INTERNET_STATUS_NAME_RESOLVED:        processed = true; std::wcerr << "--> Server address resolved to: " << ((LPCSTR)info) << std::endl; break;
			case INTERNET_STATUS_CONNECTING_TO_SERVER: processed = true; std::wcerr << "--> Connecting to server..."                        << std::endl; break;
			case INTERNET_STATUS_SENDING_REQUEST:      processed = true; std::wcerr << "--> Sending request to server..."                   << std::endl; break;
			case INTERNET_STATUS_REDIRECT:             processed = true; std::wcerr << "--> Redirecting: " << ((PCTSTR)info)                << std::endl; break;
			case INTERNET_STATUS_RECEIVING_RESPONSE:   processed = true; std::wcerr << "--> Request sent, awaiting response..."             << std::endl; break;
		}
		if(processed)
		{
			m_current_status = status;
		}
	}
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
		std::wcerr << "HttpQueryInfo() has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
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
		std::wcerr << "HttpQueryInfo() has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
	}

	return false;
}
