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

#pragma once

#include <string>
#include <stdint.h>

namespace Utils
{
	static const uint64_t TICKS_PER_SECCOND = 10000000ui64;

	std::wstring &trim(std::wstring &str);
	bool next_token(const std::wstring &str, const wchar_t *sep, std::wstring &token, size_t &offset);

	std::wstring exe_path(const std::wstring &suffix = std::wstring());
	bool file_exists(const std::wstring &path);

	void set_console_title(const std::wstring &title);

	std::wstring win_error_string(const uint32_t &error_code);
	std::wstring crt_error_string(const int      &error_code);
	std::wstring status_to_string(const uint32_t &rspns_code);

	std::wstring nbytes_to_string(const double &count);
	std::wstring second_to_string(const double &count);

	std::string wide_str_to_utf8(const std::wstring &str);
	std::wstring utf8_to_wide_str(const std::string &str);

	void trigger_system_sound(const bool &success);
	bool check_user_abort_flag(void);

	uint64_t parse_timestamp(const std::wstring &str);
	std::wstring timestamp_to_str(const uint64_t &timestamp);
	time_t decode_date_str(const char *const date_str);

	uint64_t get_file_time(const std::wstring &path);
	bool set_file_time(const int &file_no, const uint64_t &timestamp);
}


#define TRIGGER_SYSTEM_SOUND(X,Y) do \
{ \
	if((X))  \
	{ \
		Utils::trigger_system_sound((Y)); \
	} \
} \
while(0)

#define CHECK_USER_ABORT() do \
{ \
	if(Utils::check_user_abort_flag()) \
	{ \
		exit(EXIT_FAILURE); \
	} \
} \
while(0)

#if _MSC_VER >= 1800
#define ISNAN(X) std::isnan((X))
#define ROUND(X) std::round((X))
#else
#define ISNAN(X) _isnan((X))
double ROUND(const double &d);
#endif

#define DBL_VALID_GTR(X,Y) ((!ISNAN((X))) && ((X) > (Y)))
#define DBL_VALID_LSS(X,Y) ((!ISNAN((X))) && ((X) < (Y)))
#define DBL_VALID_GEQ(X,Y) ((!ISNAN((X))) && ((X) >= (Y)))
#define DBL_VALID_LEQ(X,Y) ((!ISNAN((X))) && ((X) <= (Y)))

#define DBL_TO_UINT32(X) (((X) < UINT32_MAX) ? uint32_t((X)) : UINT32_MAX)
