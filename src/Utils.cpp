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

#include "Utils.h"

//CRT
#include <sstream>

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//=============================================================================
// TRIM STRING
//=============================================================================

static inline bool is_white_space(const wchar_t c)
{
	return iswspace(c) || iswcntrl(c);
}

static std::wstring &trim_r(std::wstring &str)
{
	size_t pos = str.length();
	while(pos > 0)
	{
		if(!is_white_space(str[--pos]))
		{
			if(++pos < str.length())
			{
				str.erase(pos, std::wstring::npos);
			}
			return str;
		}
	}
	str.clear();
	return str;
}

static std::wstring &trim_l(std::wstring &str)
{
	size_t pos = 0;
	while(pos < str.length())
	{
		if(!is_white_space(str[pos++]))
		{
			if(pos > 1)
			{
				str.erase(0, --pos);
			}
			return str;
		}
	}
	str.clear();
	return str;
}

std::wstring &trim(std::wstring &str)
{
	return trim_l(trim_r(str));
}

//=============================================================================
// WIN32/WININET ERROR TO STRING
//=============================================================================

std::wstring error_string(const uint32_t &error_code)
{
	std::wstring result;
	if(error_code)
	{
		DWORD formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		HMODULE hModule = NULL;
		if((error_code >= 12000) && (error_code < 13000))
		{
			if(hModule = GetModuleHandle(L"wininet")) formatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
		}

		LPVOID lpMsgBuf = NULL;
		DWORD bufLen = FormatMessage(formatFlags, hModule, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		if((bufLen > 0) && lpMsgBuf)
		{
			result = trim(std::wstring((LPTSTR)lpMsgBuf));
			LocalFree(lpMsgBuf);
		}
	}

	if(result.empty())
	{
		std::wostringstream str;
		str << L"Unknown error (Code: " << error_code << L").";
		result = str.str();
	}
	else
	{
		const wchar_t last = result[result.length()-1];
		if((last != L'.') && (last != L'!') && (last != L'?') && (last != L',') && (last != L';'))
		{
			result += L'.';
		}
	}

	return result;
}
