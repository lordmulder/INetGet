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

#include "Thread.h"

//Win32
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//CRT
#include <cstdlib>
#include <process.h>
#include <algorithm>

//=============================================================================
// THREAD ENTRY POINT
//=============================================================================

uint32_t __stdcall Thread::thread_start(void *const data)
{
	if(Thread *const thread = ((Thread*)data))
	{
		const DWORD ret = thread->main();
		return (ret == STILL_ACTIVE) ? (ret + 1) : ret;
	}
	return DWORD(-1L);
}

//=============================================================================
// CONSTRUCTOR & DESTRUCTOR
//=============================================================================

Thread::Thread()
:
	m_signal_stop(m_event_stop)
{
	m_thread = NULL;
}

Thread::~Thread()
{
	if(is_running())
	{
		stop(1000, true);
	}
	close_handle();
}

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

bool Thread::start(void)
{
	if(is_running())
	{
		return false; /*thread already running*/
	}

	close_handle();
	m_event_stop.set(false);
	set_error_text();

	if(m_thread = _beginthreadex(NULL, 0, thread_start, this, 0, NULL))
	{
		return true;
	}

	return false; /*failed to create*/
}

bool Thread::join(const uint32_t &timeout)
{
	if(m_thread)
	{
		return (WaitForSingleObject((HANDLE) m_thread, ((timeout > 0) ? timeout : INFINITE)) == WAIT_OBJECT_0);
	}
	return false;
}

bool Thread::stop(const uint32_t &timeout, const bool &force)
{
	if(m_thread)
	{
		m_event_stop.set(true);
		if(join(std::max(1U, timeout)))
		{
			return true;
		}
		else if(force)
		{
			if(TerminateThread((HANDLE) m_thread, DWORD(-1L)))
			{
				return join();
			}
		}
	}
	return false;
}

bool Thread::is_running(void) const
{
	if(m_thread)
	{
		return (WaitForSingleObject((HANDLE) m_thread, 0) == WAIT_TIMEOUT);
	}
	return false;
}

uint32_t Thread::get_result(void) const
{
	if(m_thread)
	{
		DWORD exit_code = DWORD(-1);
		if(GetExitCodeThread((HANDLE) m_thread, &exit_code))
		{
			if(exit_code != STILL_ACTIVE)
			{
				return exit_code;
			}
		}
	}
	return DWORD(-1);
}

std::wstring Thread::get_error_text(void) const
{
	std::wstring retval;
	{
		Sync::Locker locker(m_mutex_error_txt);
		retval = m_error_text;
	}
	return retval;
}


//=============================================================================
// PROTECTED FUNCTIONS
//=============================================================================

bool Thread::is_stopped(void)
{
	return m_signal_stop.get();
}

void Thread::set_error_text(const std::wstring &text)
{
	Sync::Locker locker(m_mutex_error_txt);
	m_error_text = text.empty() ? std::wstring(L"Operation completed successfully.") : text;
}

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

void Thread::close_handle(void)
{
	if(m_thread)
	{
		CloseHandle((HANDLE) m_thread);
		m_thread = NULL;
	}
}
