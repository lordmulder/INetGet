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

#include "URL.h"

#include <Windows.h>
#include <WinInet.h>
#include <vector>

#define INIT_URL_STRING(X) ((components.dw##X##Length > 0U) ? std::wstring(components.lpsz##X, components.dw##X##Length) : std::wstring())
#define RETR_URL_STRING(X) ((LPWSTR) (m_str##X.empty() ? NULL : m_str##X.c_str()))

URL::URL(const wchar_t *const url)
:
	m_uiPortNumber(INTERNET_INVALID_PORT_NUMBER)
{
	URL_COMPONENTS components;
	SecureZeroMemory(&components, sizeof(URL_COMPONENTS));

	components.dwStructSize      = sizeof(URL_COMPONENTS);

	components.dwSchemeLength    = DWORD(-1);
	components.dwHostNameLength  = DWORD(-1);
	components.dwUserNameLength  = DWORD(-1);
	components.dwPasswordLength  = DWORD(-1);
	components.dwUrlPathLength   = DWORD(-1);
	components.dwExtraInfoLength = DWORD(-1);

	if(InternetCrackUrl(url, 0, 0, &components))
	{
		m_strScheme    = INIT_URL_STRING(Scheme);
		m_strHostName  = INIT_URL_STRING(HostName);
		m_strUserName  = INIT_URL_STRING(UserName);
		m_strPassword  = INIT_URL_STRING(Password);
		m_strUrlPath   = INIT_URL_STRING(UrlPath);
		m_strExtraInfo = INIT_URL_STRING(ExtraInfo);
		m_uiPortNumber = components.nPort;
	}
}

URL::~URL()
{
}

std::wstring URL::toString(void) const
{
	URL_COMPONENTS components;
	SecureZeroMemory(&components, sizeof(URL_COMPONENTS));

	components.dwStructSize      = sizeof(URL_COMPONENTS);

	components.dwSchemeLength    = m_strScheme   .length();
	components.dwHostNameLength  = m_strHostName .length();
	components.dwUserNameLength  = m_strUserName .length();
	components.dwPasswordLength  = m_strPassword .length();
	components.dwUrlPathLength   = m_strUrlPath  .length();
	components.dwExtraInfoLength = m_strExtraInfo.length();

	components.lpszScheme        = RETR_URL_STRING(Scheme);
	components.lpszHostName      = RETR_URL_STRING(HostName);
	components.lpszUserName      = RETR_URL_STRING(UserName);
	components.lpszPassword      = RETR_URL_STRING(Password);
	components.lpszUrlPath       = RETR_URL_STRING(UrlPath);
	components.lpszExtraInfo     = RETR_URL_STRING(ExtraInfo);
	components.nPort             = m_uiPortNumber;

	if(components.lpszScheme && (components.nPort == INTERNET_INVALID_PORT_NUMBER))
	{
		if(_wcsicmp(components.lpszScheme, L"http")   == 0) components.nPort = INTERNET_DEFAULT_HTTP_PORT;
		if(_wcsicmp(components.lpszScheme, L"https")  == 0) components.nPort = INTERNET_DEFAULT_HTTPS_PORT;
		if(_wcsicmp(components.lpszScheme, L"ftp")    == 0) components.nPort = INTERNET_DEFAULT_FTP_PORT;
		if(_wcsicmp(components.lpszScheme, L"gopher") == 0) components.nPort = INTERNET_DEFAULT_GOPHER_PORT;
	}

	DWORD len = 64;
	len += components.dwSchemeLength;
	len += components.dwHostNameLength;
	len += components.dwUserNameLength;
	len += components.dwPasswordLength;
	len += components.dwUrlPathLength;
	len += components.dwExtraInfoLength;

	std::vector<wchar_t> temp(len, L'\0');

	if(InternetCreateUrl(&components, 0, temp.data(), &len))
	{
		return std::wstring(temp.data(), len);
	}
	
	return std::wstring();
}