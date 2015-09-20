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
#include <MMSystem.h>
#include <WinInet.h>

//=============================================================================
// ROUND
//=============================================================================

#if _MSC_VER < 1800

double ROUND(const double &d)
{
	double tmp;
	modf(((d >= 0.0) ? (d + 0.5) : (d - 0.5)), &tmp);
	return tmp;
}

#endif //_MSC_VER

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
		if((error_code >= INTERNET_ERROR_BASE) && (error_code <= INTERNET_ERROR_LAST))
		{
			if(hModule = GetModuleHandle(L"wininet")) formatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
		}
		LPVOID lpMsgBuf = NULL;
		DWORD bufLen = FormatMessage(formatFlags, hModule, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		if((bufLen > 0) && lpMsgBuf)
		{
			std::wstring temp((LPTSTR)lpMsgBuf);
			result = trim(temp);
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
		std::wstring temp((LPTSTR)buffer);
		result = trim(temp);
	}

	return fixup_error_msg(result, uint32_t(error_code));
}

//=============================================================================
// STATUS CODE TO STRING
//=============================================================================

std::wstring status_to_string(const uint32_t &rspns_code)
{
	for(size_t i = 0; STATUS_CODES[i].info; i++)
	{
		if(STATUS_CODES[i].code == rspns_code)
		{
			return std::wstring(STATUS_CODES[i].info);
		}
	}
	return std::wstring(L"Unknown");
}

//=============================================================================
// STRING FORMATTING
//=============================================================================

std::wstring nbytes_to_string(const double &count)
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

std::wstring second_to_string(const double &count)
{
	const double COUNT[] =
	{
		60.0, 60.0, 24.0, 7.0, 52.0, 1.0
	};

	const wchar_t *const UNITS[] =
	{
		L"sec", L"min", L"hrs", L"dys", L"wks", L"yrs", NULL
	};

	if(count < 0.0)
	{
		return std::wstring(L"N/A");
	}

	double value = count;
	size_t index = 0;
	while((value > COUNT[index]) && (index < 5))
	{
		value /= COUNT[index++];
	}

	std::wostringstream str;
	if(index > 2)
	{
		str << std::fixed << std::setprecision(2) << std::setw(0) << std::setfill(L'0') << value << L' ' << UNITS[index];
	}
	else if(index > 0)
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

//=============================================================================
// WIDE STRING <--> UTF-8 STRING
//=============================================================================

std::string wide_str_to_utf8(const std::wstring &input)
{
	std::string result;

	if(!input.empty())
	{
		const int buff_size = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, NULL, 0, NULL, NULL);
		if(buff_size > 0)
		{
			if(char *const buffer = (char*) _malloca(sizeof(char) * buff_size))
			{
				const int retval = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, buffer, buff_size, NULL, NULL);
				if((retval > 0) && (retval <= buff_size))
				{
					result = std::string(buffer);
				}
				_freea(buffer);
			}
		}
	}

	return result;
}

std::wstring utf8_to_wide_str(const std::string &input)
{
	std::wstring result;

	if(!input.empty())
	{
		const int buff_size = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
		if(buff_size > 0)
		{
			if(wchar_t *const buffer = (wchar_t*) _malloca(sizeof(wchar_t) * buff_size))
			{
				const int retval = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, buffer, buff_size);
				if((retval > 0) && (retval <= buff_size))
				{
					result = std::wstring(buffer);
				}
				_freea(buffer);
			}
		}
	}

	return result;
}

//=============================================================================
// NOTIFICATION SOUND
//=============================================================================

void trigger_system_sound(const bool &success)
{
	PlaySound((LPCTSTR)(success ? SND_ALIAS_SYSTEMASTERISK : SND_ALIAS_SYSTEMHAND), NULL, SND_ALIAS_ID | SND_SYNC | SND_SYSTEM);
}
