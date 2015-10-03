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
#include <stdexcept>
#include <sstream>

//Helper functions
//static const wchar_t *CSTR(const std::wstring &str) { return str.empty() ? NULL : str.c_str(); }
//static const char    *CSTR(const std::string  &str) { return str.empty() ? NULL : str.c_str(); }

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

FtpClient::FtpClient(const Sync::Signal &user_aborted, const bool &disableProxy, const std::wstring &userAgentStr, const bool& /*no_redir*/, const bool& /*insecure*/, const double &timeout_con, const double &timeout_rcv, const uint32_t &connect_retry, const bool &verbose)
:
	AbstractClient(user_aborted, disableProxy, userAgentStr, timeout_con, timeout_rcv, connect_retry, verbose)
{
}

FtpClient::~FtpClient(void)
{
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool FtpClient::open(const http_verb_t& /*verb*/, const URL& /*url*/, const std::string& /*post_data*/, const std::wstring& /*referrer*/, const uint64_t& /*timestamp*/)
{
	if(!wininet_init())
	{
		return false; /*WinINet failed to initialize*/
	}

	//Close the existing connection, just to be sure
	if(!close())
	{
		emit_message(std::wstring(L"ERROR: Failed to close the existing connection!"), true);
		return false;
	}

	throw std::runtime_error("FTP support *not* implemented in this version :-(");
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

	return success;
}

//=============================================================================
// QUERY RESULT
//=============================================================================

bool FtpClient::result(bool& /*success*/, uint32_t& /*status_code*/, uint64_t& /*file_size*/, uint64_t& /*time_stamp*/, std::wstring& /*content_type*/, std::wstring& /*content_encd*/)
{
	throw std::runtime_error("FTP support *not* implemented in this version :-(");
	//return false;
}

//=============================================================================
// READ PAYLOAD
//=============================================================================

bool FtpClient::read_data(uint8_t* /*out_buff*/, const uint32_t& /*buff_size*/, size_t& /*bytes_read*/, bool& /*eof_flag*/)
{
	throw std::runtime_error("FTP support *not* implemented in this version :-(");
	//return false;
}
