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

//HTTP status codes
typedef struct
{
	const uint32_t code;
	const wchar_t *const info;
}
http_status_t;

//List of status codes
extern const http_status_t STATUS_CODES[];

//HTTP methods (verbs)
typedef enum
{
	HTTP_GET     = 0x0,
	HTTP_POST    = 0x1,
	HTTP_PUT     = 0x2,
	HTTP_DELETE  = 0x3,
	HTTP_HEAD    = 0x4,
	HTTP_UNDEF   = 0xF,
}
http_verb_t;
