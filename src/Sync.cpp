///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015-2018 LoRd_MuldeR <MuldeR2@GMX.de>
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

#include "Sync.h"

//Win32
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//CRT
#include <algorithm>
#include <stdexcept>

//=============================================================================
// MUTEX
//=============================================================================

Sync::Mutex::Mutex(void)
:
	m_handle((uintptr_t) LocalAlloc(LPTR, sizeof(RTL_CRITICAL_SECTION)))
{
	if(m_handle == NULL)
	{
		throw std::runtime_error("Failed to allocate CriticalSection object!");
	}
	InitializeCriticalSection((LPCRITICAL_SECTION) m_handle);
}

Sync::Mutex::~Mutex(void)
{
	if(m_handle)
	{
		DeleteCriticalSection((LPCRITICAL_SECTION) m_handle);
		LocalFree((HLOCAL) m_handle);
	}
}

void Sync::Mutex::enter(void)
{
	if(m_handle)
	{
		EnterCriticalSection((LPCRITICAL_SECTION) m_handle);
	}
}

void Sync::Mutex::leave(void)
{
	if(m_handle)
	{
		LeaveCriticalSection((LPCRITICAL_SECTION) m_handle);
	}
}

//=============================================================================
// MUTEX RAII-STYLE LOCKER
//=============================================================================

Sync::Locker::Locker(Mutex &mutex)
:
	m_mutex(mutex)
{
	m_mutex.enter();
}

Sync::Locker::~Locker()
{
	m_mutex.leave();
}

//=============================================================================
// EVENT
//=============================================================================

Sync::Event::Event(void)
:
	m_handle((uintptr_t) CreateEvent(NULL, TRUE, FALSE, NULL))
{
	if(m_handle == NULL)
	{
		throw std::runtime_error("Failed to allocate Event object!");
	}
}

Sync::Event::~Event(void)
{
	if(m_handle)
	{
		CloseHandle((HANDLE) m_handle);
	}
}

void Sync::Event::set(const bool &flag)
{
	if(m_handle)
	{
		if(!(flag ? SetEvent((HANDLE) m_handle) : ResetEvent((HANDLE) m_handle)))
		{
			throw std::runtime_error("Failed to set or reset Event object!");
		}
	}
}

bool Sync::Event::get(void) const
{
	if(m_handle)
	{
		return (WaitForSingleObject((HANDLE) m_handle, 0) == WAIT_OBJECT_0);
	}
	return false;
}

bool Sync::Event::await(const uint32_t &timeout) const
{
	if(m_handle)
	{
		return (WaitForSingleObject((HANDLE) m_handle, std::max(timeout, 1U)) == WAIT_OBJECT_0);
	}
	return false;
}

//=============================================================================
// SIGNAL
//=============================================================================

Sync::Signal::Signal(const Event &_event)
:
	m_event(_event)
{
}

Sync::Signal::~Signal()
{
}

bool Sync::Signal::get(void) const
{
	return m_event.get();
}

bool Sync::Signal::await(const uint32_t &timeout) const
{
	return m_event.await(timeout);
}

uintptr_t Sync::Signal::handle(void) const
{
	HANDLE retval = NULL;
	if(DuplicateHandle(GetCurrentProcess(), (HANDLE) m_event.m_handle, GetCurrentProcess(), &retval, SYNCHRONIZE, FALSE, 0))
	{
		return (uintptr_t) retval;
	}
	return NULL;
}