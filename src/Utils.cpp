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

#include "Utils.h"

//CRT
#include <sstream>
#include <iostream>
#include <io.h>
#include <iomanip>

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <MMSystem.h>
#include <WinInet.h>

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

std::wstring &Utils::trim(std::wstring &str)
{
	return trim_l(trim_r(str));
}

//=============================================================================
// TOKENIZE STRING
//=============================================================================

bool Utils::next_token(const std::wstring &str, const wchar_t *sep, std::wstring &token, size_t &offset)
{
	while(offset < str.length())
	{
		size_t sep_pos = std::wstring::npos;
		for(size_t i = 0; sep[i]; i++)
		{
			const size_t pos = str.find_first_of(sep, offset);
			sep_pos = min(sep_pos, pos);
		}
		token = str.substr(offset, (sep_pos != std::wstring::npos) ? (sep_pos - offset) : std::wstring::npos);
		offset = (sep_pos != std::wstring::npos) ? (sep_pos + 1) : std::wstring::npos;
		if(!(trim(token).empty()))
		{
			return true;
		}
	}
	return false;
}

//=============================================================================
// EXECUTABLE PATH
//=============================================================================

static std::wstring &clean_path(std::wstring &path)
{
	size_t pos = size_t(-1);
	while((pos = path.find_first_of(L'/')) != std::wstring::npos)
	{
		path[pos] = L'\\';
	}

	return path;
}

static std::wstring &add_suffix(std::wstring &path, const std::wstring &suffix)
{
	const size_t pos_sep = path.find_last_of(L'\\');
	const size_t pos_dot = path.find_last_of(L'.');

	if((pos_dot != std::wstring::npos) && ((pos_sep == std::wstring::npos) || (pos_dot > pos_sep)))
	{
		path.erase(pos_dot);
	}

	return path.append(suffix);
}

std::wstring Utils::exe_path(const std::wstring &suffix)
{
	static const size_t BUFF_SIZE = 2048;
	wchar_t buffer[BUFF_SIZE];

	const DWORD ret = GetModuleFileNameW(NULL, buffer, BUFF_SIZE);
	if((ret > 0) && (ret < BUFF_SIZE))
	{
		std::wstring temp(buffer);
		if(clean_path(trim(temp)).length() > 0)
		{
			return suffix.empty() ? temp : add_suffix(temp, suffix);
		}
	}

	return std::wstring(); /*something went wrong!*/
}

//=============================================================================
// SET CONSOLE TITLE
//=============================================================================

static volatile LONG g_console_init = 1;
static const HWND g_console_hwnd = GetConsoleWindow();
static HICON g_original_console_icon = NULL;
static std::wstring  g_original_console_title;

static bool set_console_icon(const HICON icon, HICON &original_icon)
{
	if(g_console_hwnd)
	{
		original_icon = (HICON) SendMessage(g_console_hwnd, WM_SETICON, ICON_SMALL, LPARAM(icon));
		return true;
	}
	return false;
}

static HICON load_icon(const wchar_t *const name)
{
	return (HICON) LoadImage(GetModuleHandle(NULL), name, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

static void restore_console_title()
{
	if(!g_original_console_title.empty())
	{
		SetConsoleTitleW(g_original_console_title.c_str());
	}
	if(g_original_console_icon)
	{
		HICON ignore_icon = NULL;
		set_console_icon(g_original_console_icon, ignore_icon);
	}
}

void Utils::set_console_title(const std::wstring &title)
{
	static const size_t MAX_LENGTH = 384;

	if(g_console_hwnd)
	{
		if(InterlockedExchange(&g_console_init, 0))
		{
			if(wchar_t *const buffer = (wchar_t*)_malloca(sizeof(wchar_t) * MAX_LENGTH))
			{
				if(GetConsoleTitleW(buffer, MAX_LENGTH) > 0)
				{
					g_original_console_title = std::wstring(buffer);
					atexit(restore_console_title);
				}
				_freea(buffer);
			}
			if(const HICON icon = load_icon(L"INETGET"))
			{
				set_console_icon(icon, g_original_console_icon);
			}
		}
		SetConsoleTitleW((title.length() > MAX_LENGTH) ? title.substr(0, MAX_LENGTH).c_str() : title.c_str());
	}
}

//=============================================================================
// CHECK FILE EXISTENCE
//=============================================================================

bool Utils::file_exists(const std::wstring &path)
{
	if(!path.empty())
	{
		return (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
	}

	return false;
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

std::wstring Utils::win_error_string(const uint32_t &error_code)
{
	std::wstring result;
	if(error_code)
	{
		for(size_t i = 0; i < 3; i++)
		{
			HMODULE hModule = NULL;
			DWORD formatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
			if(hModule = ((i > 0) ? GetModuleHandle((i > 1) ? L"winhttp" : L"wininet") : NULL))
			{
				formatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
			}
			LPVOID lpMsgBuf = NULL;
			DWORD bufLen = FormatMessage(formatFlags, hModule, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
			if((bufLen > 0) && lpMsgBuf)
			{
				std::wstring temp((LPTSTR)lpMsgBuf);
				result = trim(temp);
				LocalFree(lpMsgBuf);
				break;
			}
		}
	}

	return fixup_error_msg(result, error_code);
}

std::wstring Utils::crt_error_string(const int &error_code)
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

std::wstring Utils::status_to_string(const uint32_t &rspns_code)
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

std::wstring Utils::nbytes_to_string(const double &count)
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
	while((value + 0.05 >= 1000.0) && (index < 8))
	{
		index++;
		value /= 1024.0;
	}

	const size_t prec = (index > 0) ? ((value + 0.005 < 100.0) ? ((value + 0.0005 < 10.0) ? 3 : 2) : 1) : 0;
	//std::wcerr << value << L" --> " << prec << std::endl;

	std::wostringstream str;
	str << std::fixed << std::setprecision(prec) << value << L' ' << UNITS[index];
	return str.str();
}

std::wstring Utils::second_to_string(const double &count)
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
	while((value + DBL_EPSILON >= COUNT[index]) && (index < 5))
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
		str << std::fixed << std::setprecision(0) << std::setw(0) << intpart << L':' << std::setw(2) << std::setfill(L'0') << floor(fracpart * 60.0) << L' ' << UNITS[index];
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

std::string Utils::wide_str_to_utf8(const std::wstring &input)
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

std::wstring Utils::utf8_to_wide_str(const std::string &input)
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

void Utils::trigger_system_sound(const bool &success)
{
	PlaySound((LPCTSTR)(success ? SND_ALIAS_SYSTEMASTERISK : SND_ALIAS_SYSTEMHAND), NULL, SND_ALIAS_ID | SND_SYNC | SND_SYSTEM);
}

//=============================================================================
// DECODE DATE
//=============================================================================

static int month_str2int(const char *str)
{
	static const char *const MONTHS_LUT[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	int ret = 0;
	for(int j = 0; j < 12; j++)
	{
		if(!_strnicmp(str, MONTHS_LUT[j], 3))
		{
			ret = j;
			break;
		}
	}

	return ret;
}

time_t Utils::decode_date_str(const char *const date_str) //Mmm dd yyyy
{
	tm date_time;
	memset(&date_time, 0, sizeof(tm));

	if(sscanf_s(date_str, "%*3s %2d %4d", &date_time.tm_mday, &date_time.tm_year) != 2)
	{
		return 0;
	}

	date_time.tm_year -= 1900;

	if((date_time.tm_mon = month_str2int(date_str)) < 1)
	{
		return 0;
	}

	return mktime(&date_time);
}

//=============================================================================
// PARSE TIME-STAMP
//=============================================================================

static const wchar_t *const WCS_MONTH[] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec", NULL };
static const wchar_t *const WCS_WKDAY[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", NULL };

static uint64_t filetime_to_uint64(const FILETIME &filetime)
{
	ULARGE_INTEGER temp;
	temp.HighPart = filetime.dwHighDateTime;
	temp.LowPart  = filetime.dwLowDateTime;
	return temp.QuadPart;
}

static void uint64_to_filetime(const uint64_t &timestamp, FILETIME &filetime)
{
	ULARGE_INTEGER temp;
	temp.QuadPart = timestamp;
	filetime.dwHighDateTime = temp.HighPart;
	filetime.dwLowDateTime  = temp.LowPart;
}

static uint16_t parse_month_str(const std::wstring &str)
{
	for(size_t i = 0; WCS_MONTH[i]; i++)
	{
		if(!_wcsicmp(str.c_str(), WCS_MONTH[i]))
		{
			return WORD(i + 1);
		}
	}

	throw std::invalid_argument("Not a valid month!");
}

static void parse_clock_str(uint16_t &hrs, uint16_t &min, uint16_t &sec, const std::wstring &str)
{
	std::wstring token;
	size_t offset = 0;

	for(size_t i = 0; Utils::next_token(str, L":", token, offset); i++)
	{
		switch(i)
		{
			case 0: hrs = (uint16_t) std::stoul(token); break;
			case 1: min = (uint16_t) std::stoul(token); break;
			case 2: sec = (uint16_t) std::stoul(token); break;
		}
	}
}

static WORD fixup_year(const WORD &year)
{
	if(year < 100)
	{
		time_t timeval;
		if(time(&timeval))
		{
			struct tm systime;
			if(gmtime_s(&systime, &timeval) == 0)
			{
				const WORD prefix = WORD((systime.tm_year + 1900) / 100);
				return year + (100 * prefix);
			}
		}
		return 2000 + year;
	}
	return year;
}

uint64_t Utils::parse_timestamp(const std::wstring &str)
{
	if(str.empty())
	{
		return NULL; /*string is empty*/
	}

	SYSTEMTIME systime = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	std::wstring token;
	bool is_gmt = false;
	size_t offset = 0;

	try
	{
		for(size_t i = 0; Utils::next_token(str, L" ,-", token, offset); i++)
		{
			switch(i)
			{
				case 1: systime.wDay   = (WORD) std::stoul(token);                               break;
				case 2: systime.wMonth = parse_month_str(token);                                 break;
				case 3: systime.wYear  = fixup_year((WORD) std::stoul(token));                   break;
				case 4:	parse_clock_str(systime.wHour, systime.wMinute, systime.wSecond, token); break;
				case 5: is_gmt = (!_wcsicmp(token.c_str(), L"GMT"));                             break;
			}
		}
	}
	catch(std::exception&)
	{
		return NULL; /*parsing error*/
	}

	if(is_gmt)
	{
		FILETIME filetime = { 0, 0 };
		if(SystemTimeToFileTime(&systime, &filetime))
		{
			return filetime_to_uint64(filetime);
		}
	}

	return NULL;
}

std::wstring Utils::timestamp_to_str(const uint64_t &timestamp)
{
	FILETIME filetime = {0, 0 };
	SYSTEMTIME systime = { 0, 0, 0, 0, 0, 0, 0, 0 };

	uint64_to_filetime(timestamp, filetime);
	if(FileTimeToSystemTime(&filetime, &systime))
	{
		std::wostringstream str;
		str << WCS_WKDAY[min(systime.wDayOfWeek, 6U)] << L", ";
		str << std::setw(2) << std::setfill(L'0') << uint32_t(max(1, min(systime.wDay, 31U)))       << L' ';
		str << WCS_MONTH[max(1, min(systime.wMonth, 12U)) - 1]                                      << L' ';
		str << std::setw(4) << std::setfill(L'0') << uint32_t(max(1601, min(systime.wYear, 9999U))) << L' ';
		str << std::setw(2) << std::setfill(L'0') << uint32_t(min(systime.wHour, 23U))              << L':';
		str << std::setw(2) << std::setfill(L'0') << uint32_t(min(systime.wMinute, 59U))            << L':';
		str << std::setw(2) << std::setfill(L'0') << uint32_t(min(systime.wSecond, 59U))            << L' ';
		str << L"GMT";
		return str.str();
	}

	return std::wstring();
}

//=============================================================================
// GET/SET FILE TIME
//=============================================================================

bool Utils::set_file_time(const int &file_no, const uint64_t &timestamp)
{
	if(const HANDLE osHandle = (HANDLE) _get_osfhandle(file_no))
	{
		FILETIME filetime = { 0, 0 };
		uint64_to_filetime(timestamp, filetime);
		return (SetFileTime(osHandle, &filetime, NULL, &filetime) != FALSE);
	}
	return false;
}

uint64_t Utils::get_file_time(const std::wstring &path)
{
	uint64_t timestamp = NULL;
	const HANDLE osHandle = CreateFile(path.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(osHandle != INVALID_HANDLE_VALUE)
	{
		FILETIME timeCreated = { 0, 0 }, timeLastAccess = { 0, 0 }, timeLastWrite = { 0, 0 };
		if(GetFileTime(osHandle, &timeCreated, &timeLastAccess, &timeLastWrite))
		{
			timestamp = filetime_to_uint64(timeLastWrite);
		}
		CloseHandle(osHandle);
	}
	return timestamp;
}