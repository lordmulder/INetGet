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

#pragma once

#include <stdint.h>

//=============================================================================
// PROGRAM VERSION
//=============================================================================

static const uint16_t VER_INETGET_MAJOR = 1;
static const uint16_t VER_INETGET_MINOR = 0;
static const uint16_t VER_INETGET_PATCH = 0;

//=============================================================================
// BUILD INFORMATION
//=============================================================================

static const char *BUILD_DATE = __DATE__;
static const char *BUILD_TIME = __TIME__;

#if defined(_MSC_VER)
	#define _MSC_VERSION_HELPER(X,Y) X #Y
	#define _MSC_VERSION_STRING(X,Y) _MSC_VERSION_HELPER(X,Y)
	static const char *BUILD_COMP = _MSC_VERSION_STRING("Visual C++ v", _MSC_VER);
	#ifdef _M_X64
		static const char *BUILD_ARCH = "x64";
	#else
		static const char *BUILD_ARCH = "x86";
	#endif
	#undef _MSC_VERSION_STRING
	#undef _MSC_VERSION_HELPER
#else
	#error Unsupported compiler detected!
#endif
