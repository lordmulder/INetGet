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

HttpClient::HttpClient(void)
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
		std::wcerr << "HttpOpenRequest() has failed: " << error_string(error_code) << L'\n' << std::endl;
	}

	//Try to actually send the HTTP request
	if(m_hRequest != NULL)
	{
		BOOL success = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
		if(success != TRUE)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "HttpOpenRequest() has failed: " << error_string(error_code) << L'\n' << std::endl;
		}
		return (success == TRUE);
	}

	return false;
}

//
