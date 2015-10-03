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

namespace Sync
{
	class Mutex
	{
		friend class Locker;

	public:
		Mutex(void);
		~Mutex(void);

	protected:
		void enter(void);
		void leave(void);

	private:
		const uintptr_t m_handle;
	};

	class Locker
	{
	public:
		Locker(Mutex &mutex);
		~Locker(void);

	private:
		Mutex &m_mutex;
	};

	class Event
	{
		friend class Signal;

	public:
		Event(void);
		~Event(void);

		void set(const bool &flag);

	protected:
		bool get(void) const;
		bool await(const uint32_t &timeout) const;

	private:
		const uintptr_t m_handle;
	};

	class Signal
	{
	public:
		Signal(const Event &_event);
		~Signal();

		bool get(void) const;
		bool await(const uint32_t &timeout) const;

	private:
		const Event &m_event;
	};
}
