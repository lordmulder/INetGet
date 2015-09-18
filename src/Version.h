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

//=============================================================================
// PROGRAM VERSION
//=============================================================================

#define VER_INETGET_MAJOR 1
#define VER_INETGET_MIN_H 0
#define VER_INETGET_MIN_L 0
#define VER_INETGET_PATCH 0

//=============================================================================
// Helper macros (aka: having fun with the C pre-processor)
//=============================================================================

#ifdef INETGET_RCC

#define VER_INETGET_STR_HLP1(X)       #X
#define VER_INETGET_STR_HLP2(W,X,Y,Z) VER_INETGET_STR_HLP1(v##W.X##Y-Z)
#define VER_INETGET_STR_HLP3(W,X,Y,Z) VER_INETGET_STR_HLP2(W,X,Y,Z)
#define VER_INETGET_STR               VER_INETGET_STR_HLP3(VER_INETGET_MAJOR,VER_INETGET_MIN_H,VER_INETGET_MIN_L,VER_INETGET_PATCH)

#define VER_INETGET_MINOR_HLP1(X,Y)   X##Y
#define VER_INETGET_MINOR_HLP2(X,Y)   VER_INETGET_MINOR_HLP1(X,Y)
#define VER_INETGET_MINOR             VER_INETGET_MINOR_HLP2(VER_INETGET_MIN_H,VER_INETGET_MIN_L)

#endif //INETGET_RCC

//=============================================================================
// BUILD INFORMATION
//=============================================================================

#ifndef INETGET_RCC
#include <stdint.h>

#if (defined(NDEBUG) && defined(_DEBUG)) || ((!defined(NDEBUG)) && (!defined(_DEBUG)))
#error Inconsistent DEBUG flags!
#endif

static const char *BUILD_DATE = __DATE__;
static const char *BUILD_TIME = __TIME__;

#ifdef NDEBUG
static const char *BUILD_CONF = "Release";
#else
static const char *BUILD_CONF = "DEBUG";
#endif

static const uint32_t VERSION_MAJOR = VER_INETGET_MAJOR;
static const uint32_t VERSION_MINOR = VER_INETGET_MIN_L + (VER_INETGET_MIN_H * 10);
static const uint32_t VERSION_PATCH = VER_INETGET_PATCH;

#if defined(_MSC_VER)
	#if(_MSC_VER == 1900)
		static const char *BUILD_COMP = "MSVC 14.0";
	#elif(_MSC_VER == 1800)
		static const char *BUILD_COMP = "MSVC 12.0";
	#elif(_MSC_VER == 1700)
		static const char *BUILD_COMP = "MSVC 11.0";
	#elif(_MSC_VER == 1600)
		static const char *BUILD_COMP = "MSVC 10.0";
	#else
		#error Unsupported compiler version!
	#endif

	#ifdef _M_X64
		static const char *BUILD_ARCH = "x64";
	#else
		static const char *BUILD_ARCH = "x86";
	#endif
#else
	#error Unsupported compiler detected!
#endif

#undef VER_INETGET_MAJOR
#undef VER_INETGET_MIN_H
#undef VER_INETGET_MIN_L
#undef VER_INETGET_PATCH

#endif //INETGET_RCC
