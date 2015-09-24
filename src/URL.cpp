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

#include "URL.h"

#include <Windows.h>
#include <WinINet.h>
#include <vector>
#include <sstream>
#include <iomanip>

#include "Utils.h"

//=============================================================================
// HELPER MACROS
//=============================================================================

#define INIT_URL_STRING_RAW(X) do \
{ \
	m_str##X.clear(); \
	if(components.dw##X##Length > 0U) \
	{ \
		std::wstring temp(components.lpsz##X, components.dw##X##Length); \
		m_str##X = trim(temp); \
	} \
} \
while(0)

#define INIT_URL_STRING_ENC(X) do \
{ \
	m_str##X.clear(); \
	if(components.dw##X##Length > 0U) \
	{ \
		std::wstring temp(components.lpsz##X, components.dw##X##Length); \
		m_str##X = utf8_to_wide_str(urlEncode(trim(temp))); \
	} \
} \
while(0)

//=============================================================================
// CONSTRUCTOR
//=============================================================================

URL::URL(const URL &other)
:
	m_iSchemeId(other.m_iSchemeId),
	m_uiPortNumber(other.m_uiPortNumber)
{
	m_strScheme    = other.m_strScheme;
	m_strHostName  = other.m_strHostName;
	m_strUserName  = other.m_strUserName;
	m_strPassword  = other.m_strPassword;
	m_strUrlPath   = other.m_strUrlPath;
	m_strExtraInfo = other.m_strExtraInfo;
}

URL::URL(const std::wstring &url)
:
	m_iSchemeId(INTERNET_SCHEME_UNKNOWN),
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
		INIT_URL_STRING_RAW(Scheme);
		INIT_URL_STRING_RAW(HostName);
		INIT_URL_STRING_RAW(UserName);
		INIT_URL_STRING_RAW(Password);
		INIT_URL_STRING_ENC(UrlPath);
		INIT_URL_STRING_ENC(ExtraInfo);
		m_iSchemeId = int16_t(components.nScheme);
		m_uiPortNumber = components.nPort;
	}
}

URL::~URL()
{
}

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

std::wstring URL::toString(void) const
{
	std::wostringstream result;
	result << m_strScheme;
	result << L"://";
	result << m_strHostName;
	result << m_strUrlPath;
	result << m_strExtraInfo;
	return result.str();
}

bool URL::isComplete(void) const
{
	if(m_strHostName.empty() || m_strUrlPath .empty())
	{
		return false;
	}
	return (m_iSchemeId > 0);
}

//=============================================================================
// STATIC FUNCTIONS
//=============================================================================

std::string URL::urlEncode(const std::wstring &url)
{
	const std::string url_utf8 = wide_str_to_utf8(url);
	return url_utf8.empty() ? std::string() : urlEncode(url_utf8);
}

std::string URL::urlEncode(const std::string &url)
{
	static const char *const ALLOWED_CHARS = "!#$&'()*+,-./:;=?@[]_~";

	std::ostringstream result;
	for(std::string::const_iterator iter = url.cbegin(); iter != url.cend(); iter++)
	{	
		if(isalnum(*iter) || strchr(ALLOWED_CHARS, (*iter)))
		{
			result << (*iter);
			continue;
		}
		const std::ios::fmtflags backup(result.flags());
		result << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<uint32_t>(static_cast<uint8_t>(*iter));
		result.flags(backup);
	}

	return result.str();
}
