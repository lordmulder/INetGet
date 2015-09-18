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

#include "Sink_Null.h"

//Internal
#include "Utils.h"

//CRT
#include <cstdio>
#include <iostream>

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

NullSink::NullSink(void)
:
	m_isOpen(false)
{
}

NullSink::~NullSink(void)
{
	close();
}

//=============================================================================
// OPEN / CLOSE
//=============================================================================

bool NullSink::open(void)
{
	m_isOpen  = true;
	return true;
}

bool NullSink::close(void)
{
	m_isOpen  = false;
	return true;
}

//=============================================================================
// WRITE
//=============================================================================

bool NullSink::write(uint8_t *const buffer, const size_t &count)
{
	if(m_isOpen)
	{
		return true;
	}
	return false;
}
