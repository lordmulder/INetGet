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

#include "Utils.h"

#define INIT_URL_STRING(X) ((components.dw##X##Length > 0U) ? trim(std::wstring(components.lpsz##X, components.dw##X##Length)) : std::wstring())

URL::URL(void)
:
	m_iSchemeId(INTERNET_SCHEME_UNKNOWN),
	m_uiPortNumber(INTERNET_INVALID_PORT_NUMBER)
{
	/*nothing to do*/
}

URL::URL(const std::wstring &url)
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

	if(InternetCrackUrl(url.c_str(), 0, 0, &components))
	{
		m_strScheme    = INIT_URL_STRING(Scheme);
		m_strHostName  = INIT_URL_STRING(HostName);
		m_strUserName  = INIT_URL_STRING(UserName);
		m_strPassword  = INIT_URL_STRING(Password);
		m_strUrlPath   = INIT_URL_STRING(UrlPath);
		m_strExtraInfo = INIT_URL_STRING(ExtraInfo);
		m_uiPortNumber = components.nPort;
		m_iSchemeId    = components.nScheme;
	}
}

URL::~URL()
{
}

bool URL::isComplete(void) const
{
	if(m_strHostName.empty() || m_strUrlPath .empty())
	{
		return false;
	}
	return (m_iSchemeId > 0);
}
