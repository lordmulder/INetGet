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

#include "Client_FTP.h"

//Internal
#include "URL.h"
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinINet.h>

//CRT
#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

//Helper functions
static const wchar_t *CSTR(const std::wstring &str) { return str.empty() ? NULL : str.c_str(); }
static const char    *CSTR(const std::string  &str) { return str.empty() ? NULL : str.c_str(); }

//Const
static const wchar_t *const HTTP_VER_11      = L"HTTP/1.1";
static const wchar_t *const ACCEPTED_TYPES[] = { L"*/*", NULL };
static const wchar_t *const TYPE_FORM_DATA   = L"Content-Type: application/x-www-form-urlencoded";

//Macros
#define OPTIONAL_FLAG(X,Y,Z) do \
{ \
	if((Y)) { (X) |= (Z); } \
} \
while(0)

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

FtpClient::FtpClient(const bool &disableProxy, const std::wstring &userAgentStr, const bool &verbose)
:
	AbstractClient(disableProxy, userAgentStr, verbose)
{
}

FtpClient::~FtpClient(void)
{
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool FtpClient::open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer, const bool &no_redir, const bool &insecure)
{
	if(!wininet_init())
	{
		return false; /*WinINet failed to initialize*/
	}

	//Close the existing connection, just to be sure
	if(!close())
	{
		std::wcerr << L"ERROR: Failed to close the existing connection!\n" << std::endl;
		return false;
	}

	throw std::runtime_error("FTP support *not* implemented in this version :-(");
	return false;
}

bool FtpClient::close(void)
{
	bool success = true;

	//Close the request, if it currently exists
	/*if(!close_handle(m_hRequest))
	{
		success = false;
	}*/

	//Close connection, if it is currently open
	/*if(!close_handle(m_hConnection))
	{
		success = false;
	}*/

	CHECK_USER_ABORT();
	return success;
}

//=============================================================================
// QUERY RESULT
//=============================================================================

bool FtpClient::result(bool &success, uint32_t &status_code, uint64_t &file_size, std::wstring &content_type, std::wstring &content_encd)
{
	throw std::runtime_error("FTP support *not* implemented in this version :-(");
	return false;
}

//=============================================================================
// READ PAYLOAD
//=============================================================================

bool FtpClient::read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag)
{
	throw std::runtime_error("FTP support *not* implemented in this version :-(");
	return false;
}
