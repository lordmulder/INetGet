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

#include "Client_Abstract.h"

//Internal
#include "Compat.h"
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinINet.h>

//CRT
#include <sstream>
#include <cmath>
#include <cfloat>

//Default User Agent string
static const wchar_t *const USER_AGENT = L"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9) Gecko/2008062901 IceWeasel/3.0"; /*use something unobtrusive*/

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

AbstractClient::AbstractClient(const Sync::Signal &user_aborted, const bool &disable_proxy, const std::wstring &agent_str, const double &timeout_con, const double &timeout_rcv, const uint32_t &connect_retry, const bool &verbose)
:
	m_user_aborted(user_aborted),
	m_disable_proxy(disable_proxy),
	m_timeout_con(timeout_con),
	m_timeout_rcv(timeout_rcv),
	m_agent_str(agent_str),
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
		m_hInternet = InternetOpen(m_agent_str.empty() ? USER_AGENT : m_agent_str.c_str(), m_disable_proxy ? INTERNET_OPEN_TYPE_DIRECT : INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(m_hInternet == NULL)
		{
			const DWORD error_code = GetLastError();
			set_error_text(std::wstring(L"InternetOpen() has failed:\n").append(Utils::win_error_string(error_code)));
			return false;
		}

		//Query version info
		DWORD versionInfoSize = sizeof(INTERNET_VERSION_INFO);
		INTERNET_VERSION_INFO versionInfo;
		if(m_verbose && InternetQueryOption(m_hInternet, INTERNET_OPTION_VERSION, &versionInfo, &versionInfoSize))
		{
			std::wostringstream version; version << versionInfo.dwMajorVersion << L'.' << versionInfo.dwMinorVersion;
			emit_message(std::wstring(L"Using WinINet API library version ").append(version.str()));
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
			static const uint32_t OPTS[] =
			{
				INTERNET_OPTION_RECEIVE_TIMEOUT, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
				INTERNET_OPTION_SEND_TIMEOUT, INTERNET_OPTION_DATA_SEND_TIMEOUT, NULL
			};
			for(size_t i = 0; OPTS[i]; i++)
			{
				if(!set_inet_options(m_hInternet, OPTS[i], DBL_TO_UINT32(rcv_timeout)))
				{
					return false; /*failed to setup timeout!*/
				}
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
// ERROR MESSAGE
//=============================================================================

std::wstring AbstractClient::get_error_text(void) const
{
	std::wstring retval;
	{
		Sync::Locker locker(m_mutex_error_txt);
		retval = m_error_text;
	}
	return retval;
}

void AbstractClient::set_error_text(const std::wstring &text)
{
	Sync::Locker locker(m_mutex_error_txt);
	m_error_text = text.empty() ? std::wstring(L"Operation completed successfully.") : text;
}

//=============================================================================
// LISTENER SUPPORT
//=============================================================================

void AbstractClient::add_listener(AbstractListener &callback)
{
	Sync::Locker locker(m_mutex_listeners);
	m_listeners.insert(&callback);
}

void AbstractClient::emit_message(const std::wstring message)
{
	Sync::Locker locker(m_mutex_listeners);
	for(std::set<AbstractListener*>::const_iterator iter = m_listeners.cbegin(); iter != m_listeners.cend(); iter++)
	{
		(*iter)->onMessage(message);
	}
}

//=============================================================================
// STATUS CALLBACK
//=============================================================================

#define CHECK_STATUS(X) do \
{ \
	if(status == (X)) \
	{ \
		static const wchar_t *name = L#X; \
		emit_message(std::wstring(&name[16])); \
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
		set_error_text(std::wstring(L"InternetSetOption() function has failed:\n").append(Utils::win_error_string(error_code)));
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
		set_error_text(std::wstring(L"InternetQueryOption() function has failed:\n").append(Utils::win_error_string(error_code)));
		return false;
	}
	return (buff_len >= sizeof(uint32_t));
}

std::wstring AbstractClient::status_str(const uintptr_t &info)
{
	if(const char *const chr_data = (const char*) info)
	{
		if(isprint(chr_data[0]) && isprint(chr_data[1]))
		{
			std::wstring temp(Utils::utf8_to_wide_str(std::string(chr_data)));
			if(Utils::trim(temp).length() > 0)
			{
				return temp;
			}
		}
	}

	if(const wchar_t *const wcs_data = (const wchar_t*) info)
	{
		if(iswprint(wcs_data[0]) && iswprint(wcs_data[1]))
		{
			std::wstring temp(wcs_data);
			if(Utils::trim(temp).length() > 0)
			{
				return temp;
			}
		}
	}

	return std::wstring(L"<N/A>");
}
