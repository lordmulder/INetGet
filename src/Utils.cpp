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

//Intenral
#include "Types.h"

//CRT
#include <sstream>
#include <iostream>
#include <iomanip>

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

static std::wstring &fixup_error_msg(std::wstring &error_msg, const uint32_t &error_code)
{
	if(error_msg.empty())
	{
		std::wostringstream str;
		str << L"Unknown error (Code: " << error_code << L").";
		error_msg = str.str();
	}
	else
	{
		const wchar_t last = error_msg[error_msg.length()-1];
		if((last != L'.') && (last != L'!') && (last != L'?') && (last != L',') && (last != L';'))
		{
			error_msg += L'.';
		}
	}
	return error_msg;
}

std::wstring win_error_string(const uint32_t &error_code)
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

	return fixup_error_msg(result, error_code);
}

std::wstring crt_error_string(const int &error_code)
{
	std::wstring result;
	static const size_t BUFFSIZE = 2048;
	wchar_t buffer[BUFFSIZE];
	if(_wcserror_s(buffer, BUFFSIZE, error_code) == 0)
	{
		result = trim(std::wstring(buffer));
	}

	return fixup_error_msg(result, uint32_t(error_code));
}

//=============================================================================
// STATUS CODE TO STRING
//=============================================================================

std::wstring status_to_string(const uint32_t &status_code)
{
	for(size_t i = 0; STATUS_CODES[i].info; i++)
	{
		if(STATUS_CODES[i].code == status_code)
		{
			return std::wstring(STATUS_CODES[i].info);
		}
	}
	return std::wstring(L"Unknown");
}

//=============================================================================
// STRING FORMATTING
//=============================================================================

std::wstring bytes_to_string(const double &count)
{
	const wchar_t *const UNITS[] =
	{
		L"Byte", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB", NULL
	};

	if(count < 0.0)
	{
		return std::wstring(L"N/A");
	}

	double value = count;
	size_t index = 0;
	while((value > 1000.0) && (index < 8))
	{
		index++;
		value /= 1024.0;
	}

	const size_t prec = (index > 0) ? ((value < 100.0) ? ((value < 10.0) ? 3 : 2) : 1) : 0;

	std::wostringstream str;
	str << std::fixed << std::setprecision(prec) << value << L' ' << UNITS[index];
	return str.str();
}

std::wstring ticks_to_string(const double &count)
{
	const wchar_t *const UNITS[] =
	{
		L"sec", L"min", L"hrs", NULL
	};

	if(count < 0.0)
	{
		return std::wstring(L"N/A");
	}

	double value = count;
	size_t index = 0;
	while((value > 60.0) && (index < 2))
	{
		index++;
		value /= 60.0;
	}

	std::wostringstream str;
	if(index > 0)
	{
		double intpart; const double fracpart = modf(value, &intpart);
		str << std::fixed << std::setprecision(0) << std::setw(0) << intpart << L':' << std::setw(2) << std::setfill(L'0') << (fracpart * 60.0) << L' ' << UNITS[index];
	}
	else
	{
		str << std::fixed << std::setprecision(0) << value << L' ' << UNITS[index];
	}
	return str.str();
}
