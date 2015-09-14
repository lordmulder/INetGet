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

#include "Timer.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//CRT
#include <stdexcept>

static inline uint64_t query_frequency(void)
{
	LARGE_INTEGER frequency;
	if(!QueryPerformanceFrequency(&frequency))
	{
		throw std::runtime_error("QueryPerformanceFrequency() has failed!");
	}
	return (uint64_t) max(frequency.QuadPart, 0i64);
}

static inline uint64_t query_timer(void)
{
	LARGE_INTEGER timer;
	if(!QueryPerformanceCounter(&timer))
	{
		throw std::runtime_error("QueryPerformanceCounter() has failed!");
	}
	return (uint64_t) max(timer.QuadPart, 0i64);
}

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

Timer::Timer()
:
	m_frequency(query_frequency())
{
	reset();
}

Timer::~Timer()
{
}

//=============================================================================
// TIMER FUNCTIONS
//=============================================================================

void Timer::reset(void)
{
	m_reference = query_timer();
}

double Timer::query(void)
{
	return double(query_timer() - m_reference) / double(m_frequency);
}
