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

#include "Client_Abstract.h"

//Internal
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinINet.h>

//CRT
#include <iostream>
#include <cmath>
#include <cfloat>

//Default User Agent string
static const wchar_t *const USER_AGENT = L"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9) Gecko/2008062901 IceWeasel/3.0"; /*use something unobtrusive*/

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

AbstractClient::AbstractClient(const bool &disableProxy, const std::wstring &userAgentStr, const double &timeout_con, const double &timeout_rcv, const uint32_t &connect_retry, const bool &verbose)
:
	m_timeout_con(timeout_con),
	m_timeout_rcv(timeout_rcv),
	m_disableProxy(disableProxy),
	m_userAgentStr(userAgentStr),
	m_connect_retry(connect_retry),
	m_verbose(verbose),
	m_hInternet(NULL)
{
}

AbstractClient::~AbstractClient()
{
	wininet_exit();
}

//=============================================================================
// INITIALIZE OR EXIT CLIENT
//=============================================================================

bool AbstractClient::wininet_init()
{
	if(m_hInternet == NULL)
	{
		m_hInternet = InternetOpen(m_userAgentStr.empty() ? USER_AGENT : m_userAgentStr.c_str(), m_disableProxy ? INTERNET_OPEN_TYPE_DIRECT : INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(m_hInternet == NULL)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "InternetOpen() has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		}

		//Setup the connection and receive timeouts
		if(DBL_VALID_GTR(m_timeout_con, 0.0))
		{
			const double con_timeout = (m_timeout_con < DBL_MAX) ? ROUND(1000.0 * m_timeout_con) : double(UINT32_MAX);
			if(!set_inet_options(m_hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, DBL_TO_UINT32(con_timeout)))
			{
				return false; /*failed to setup timeout!*/
			}
		}
		if(DBL_VALID_GTR(m_timeout_rcv, 0.0))
		{
			const double rcv_timeout = (m_timeout_rcv < DBL_MAX) ? ROUND(1000.0 * m_timeout_rcv) : double(UINT32_MAX);
			if(!set_inet_options(m_hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, DBL_TO_UINT32(rcv_timeout)))
			{
				return false; /*failed to setup timeout!*/
			}
			if(!set_inet_options(m_hInternet, INTERNET_OPTION_SEND_TIMEOUT, DBL_TO_UINT32(rcv_timeout)))
			{
				return false; /*failed to setup timeout!*/
			}
		}
	}

	return (m_hInternet != NULL);
}

bool AbstractClient::wininet_exit(void)
{
	return close_handle(m_hInternet);
}

//=============================================================================
// STATUS CALLBACK
//=============================================================================

#define CHECK_STATUS(X) do \
{ \
	if(status == (X)) \
	{ \
		static const char *name = #X; \
		std::wcerr << L"--> " << &name[16] << std::endl; \
		return; \
	} \
} \
while(0)

void __stdcall AbstractClient::status_callback(void* /*hInternet*/, uintptr_t dwContext, uint32_t dwInternetStatus, void *lpvStatusInformation, uint32_t /*dwStatusInformationLength*/)
{
	if(AbstractClient *const instance = reinterpret_cast<AbstractClient*>(dwContext))
	{
		instance->update_status(dwInternetStatus, uintptr_t(lpvStatusInformation));
	}
}

void AbstractClient::update_status(const uint32_t &status, const uintptr_t& /*information*/)
{
	if(m_verbose)
	{
		CHECK_STATUS(INTERNET_STATUS_RESOLVING_NAME);
		CHECK_STATUS(INTERNET_STATUS_NAME_RESOLVED);
		CHECK_STATUS(INTERNET_STATUS_CONNECTING_TO_SERVER);
		CHECK_STATUS(INTERNET_STATUS_CONNECTED_TO_SERVER);
		CHECK_STATUS(INTERNET_STATUS_SENDING_REQUEST);
		CHECK_STATUS(INTERNET_STATUS_REQUEST_SENT);
		CHECK_STATUS(INTERNET_STATUS_RECEIVING_RESPONSE);
		CHECK_STATUS(INTERNET_STATUS_RESPONSE_RECEIVED);
		CHECK_STATUS(INTERNET_STATUS_CTL_RESPONSE_RECEIVED);
		CHECK_STATUS(INTERNET_STATUS_PREFETCH);
		CHECK_STATUS(INTERNET_STATUS_CLOSING_CONNECTION);
		CHECK_STATUS(INTERNET_STATUS_CONNECTION_CLOSED);
		CHECK_STATUS(INTERNET_STATUS_HANDLE_CREATED);
		CHECK_STATUS(INTERNET_STATUS_HANDLE_CLOSING);
		CHECK_STATUS(INTERNET_STATUS_DETECTING_PROXY);
		CHECK_STATUS(INTERNET_STATUS_REQUEST_COMPLETE);
		CHECK_STATUS(INTERNET_STATUS_REDIRECT);
		CHECK_STATUS(INTERNET_STATUS_INTERMEDIATE_RESPONSE);
		CHECK_STATUS(INTERNET_STATUS_USER_INPUT_REQUIRED);
		CHECK_STATUS(INTERNET_STATUS_STATE_CHANGE);
		CHECK_STATUS(INTERNET_STATUS_COOKIE_SENT);
		CHECK_STATUS(INTERNET_STATUS_COOKIE_RECEIVED);
		CHECK_STATUS(INTERNET_STATUS_PRIVACY_IMPACTED);
		CHECK_STATUS(INTERNET_STATUS_P3P_HEADER);
		CHECK_STATUS(INTERNET_STATUS_P3P_POLICYREF);
		CHECK_STATUS(INTERNET_STATUS_COOKIE_HISTORY);
		CHECK_STATUS(INTERNET_STATE_CONNECTED);
		CHECK_STATUS(INTERNET_STATE_DISCONNECTED);
		CHECK_STATUS(INTERNET_STATE_DISCONNECTED_BY_USER);
		CHECK_STATUS(INTERNET_STATE_IDLE);
		CHECK_STATUS(INTERNET_STATE_BUSY);
	}
}

//=============================================================================
// UTILITIES
//=============================================================================

bool AbstractClient::close_handle(void *&handle)
{
	bool success = true;

	if(handle != NULL)
	{
		if(InternetCloseHandle(handle) != TRUE)
		{
			success = false;
		}
		handle = NULL;
	}

	return success;
}

bool AbstractClient::set_inet_options(void *const request, const uint32_t &option, const uint32_t &value)
{
	if(!InternetSetOption(request, option, (LPVOID)&value, sizeof(uint32_t)))
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "--> Failed!\n\nInternetSetOption() function has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}
	return true;
}

bool AbstractClient::get_inet_options(void *const request, const uint32_t &option, uint32_t &value)
{
	DWORD buff_len = sizeof(uint32_t);
	if(!InternetQueryOption(request, option, (LPVOID)&value, &buff_len))
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "--> Failed!\n\nInternetQueryOption() function has failed:\n" << win_error_string(error_code) << L'\n' << std::endl;
		return false;
	}
	return (buff_len >= sizeof(uint32_t));
}
