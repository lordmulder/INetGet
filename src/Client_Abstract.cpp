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

#include "Client_Abstract.h"

//Internal
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinInet.h>

//CRT
#include <iostream>

//Default User Agent string
static const wchar_t *const USER_AGENT = L"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9) Gecko/2008062901 IceWeasel/3.0"; /*use something unobtrusive*/

//Helper macro
static const wchar_t *CSTR(const std::wstring &str) { return str.empty() ? NULL : str.c_str(); }

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

AbstractClient::AbstractClient(const bool &verbose)
:
	m_verbose(verbose),
	m_hInternet(NULL),
	m_hConnection(NULL),
	m_hRequest(NULL)
{
}

AbstractClient::~AbstractClient()
{
	client_exit();
}

//=============================================================================
// INITIALIZE OR EXIT CLIENT
//=============================================================================

bool AbstractClient::client_init(const bool &disableProxy, const std::wstring &userAgent)
{
	if(m_hInternet == NULL)
	{
		m_hInternet = InternetOpen(userAgent.empty() ? USER_AGENT : userAgent.c_str(), disableProxy ? INTERNET_OPEN_TYPE_DIRECT : INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(m_hInternet == NULL)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "InternetOpen() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
		}
	}

	return (m_hInternet != NULL);
}

bool AbstractClient::client_exit(void)
{
	//Close connection, just to be sure!
	connection_exit();

	//Close the client, if it is open
	BOOL success = TRUE;
	if(m_hInternet != NULL)
	{
		BOOL success = InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
	return (success == TRUE);
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool AbstractClient::connection_init(const uint32_t &serviceId, const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password)
{
	if(m_hInternet == NULL)
	{
		throw std::runtime_error("WinInet API not initialized yet!");
	}

	//Close existing connection, just to be sure
	if(!connection_exit())
	{
		std::wcerr << "ERROR: Failed to close the existing connection!\n" << std::endl;
		return false;
	}

	//Try to open the new connection
	m_hConnection = InternetConnect(m_hInternet, CSTR(hostName), portNo, CSTR(userName), CSTR(password), serviceId, 0, reinterpret_cast<intptr_t>(this));
	if(m_hConnection == NULL)
	{
		const DWORD error_code = GetLastError();
		std::wcerr << "InternetConnect() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
	}

	//Install the callback handler (only in verbose mode)
	if(m_verbose && (m_hConnection != NULL))
	{
		if(InternetSetStatusCallback(m_hConnection, (INTERNET_STATUS_CALLBACK)(&callback_handler)) == INTERNET_INVALID_STATUS_CALLBACK)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "InternetSetStatusCallback() has failed:\n" << error_string(error_code) << L'\n' << std::endl;
		}
	}

	return (m_hConnection != NULL);
}

bool AbstractClient::connection_exit(void)
{
	//Close request, just to be sure!
	request_exit();

	//Close connection, if it is open
	BOOL success = TRUE;
	if(m_hConnection != NULL)
	{
		BOOL success = InternetCloseHandle(m_hConnection);
		m_hConnection = NULL;
	}
	return (success == TRUE);
}

//=============================================================================
// REQUEST HANDLING
//=============================================================================

bool AbstractClient::request_exit(void)
{
	//Close request, if it is open
	BOOL success = TRUE;
	if(m_hRequest != NULL)
	{
		BOOL success = InternetCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}
	return (success == TRUE);
}

//=============================================================================
// STATUS CALLBACK
//=============================================================================

#define CHECK_STATUS(X) do \
{ \
	if(dwInternetStatus == X) \
	{ \
		static const char *name = #X; \
		std::wcerr << L"--> Internet status: " << &name[16] << std::endl; \
		return; \
	} \
} \
while(0)

void __stdcall AbstractClient::callback_handler(void *hInternet, uintptr_t dwContext, uint32_t dwInternetStatus, void *lpvStatusInformation, uint32_t dwStatusInformationLength)
{
	if(AbstractClient *const instance = reinterpret_cast<AbstractClient*>(dwContext))
	{
		instance->update_status(dwInternetStatus, ((dwStatusInformationLength == sizeof(DWORD)) ? (*reinterpret_cast<DWORD*>(lpvStatusInformation)) : DWORD(-1)));
	}
}

void AbstractClient::update_status(const uint32_t &dwInternetStatus, const uint32_t &lpvStatusInformation)
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
