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

#pragma once

#include <string>
#include <stdint.h>

extern volatile bool g_userAbortFlag;
static const uint64_t TICKS_PER_SECCOND = 10000000ui64;

std::wstring &trim(std::wstring &str);

std::wstring win_error_string(const uint32_t &error_code);
std::wstring crt_error_string(const int      &error_code);
std::wstring status_to_string(const uint32_t &rspns_code);

std::wstring nbytes_to_string(const double &count);
std::wstring second_to_string(const double &count);

std::string wide_str_to_utf8(const std::wstring &str);
std::wstring utf8_to_wide_str(const std::string &str);

void trigger_system_sound(const bool &success);

#define CHECK_USER_ABORT() do \
{ \
	if(g_userAbortFlag)  \
	{ \
		std::wcerr << L"\n\nSIGINT: Operation aborted by user !!!\n" << std::endl; \
		for(;;) exit(EXIT_FAILURE); \
	} \
} \
while(0)

#define TRIGGER_SYSTEM_SOUND(X,Y) do \
{ \
	if((X))  \
	{ \
		trigger_system_sound((Y)); \
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
