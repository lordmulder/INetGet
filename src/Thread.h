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

#include <stdint.h>
#include <string>

#include "Sync.h"

class Thread
{
public:
	Thread();
	~Thread();

	//Start
	bool start(void);

	//Stop
	bool join(const uint32_t &timeout = 0U);
	bool stop(const uint32_t &timeout = 0U, const bool &force = false);

	//Info
	bool is_running(void) const;
	std::wstring get_error_text(void) const;
	uint32_t get_result(void) const;

private:
	void close_handle(void);
	static uint32_t __stdcall thread_start(void *const data);

	Sync::Event m_event_stop;
	Sync::Signal m_signal_stop;

	mutable Sync::Mutex m_mutex_error_txt;
	std::wstring m_error_text;

	uintptr_t m_thread;

protected:
	virtual uint32_t main(void) = 0;

	void set_error_text(const std::wstring &text = std::wstring());
	bool is_stopped(void);
};

