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

#pragma once

#include "Types.h"
#include "Sync.h"

class URL;

#include <stdint.h>
#include <set>
#include <string>

class AbstractListener
{
	friend class AbstractClient;
protected:
	virtual void onMessage(const std::wstring message) = 0;
};

class AbstractClient
{
public:
	static const uint64_t SIZE_UNKNOWN = UINT64_MAX;
	static const uint64_t TIME_UNKNOWN = 0;

	AbstractClient(const Sync::Signal &user_aborted, const bool &disable_proxy = false, const std::wstring &agent_str = std::wstring(), const double &timeout_con = -1.0, const double &timeout_rcv = -1.0, const uint32_t &connect_retry = 3, const bool &verbose = false);
	virtual ~AbstractClient(void);

	//Add listener
	void add_listener(AbstractListener &callback);

	//Connection handling
	virtual bool open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer, const uint64_t &timestamp) = 0;
	virtual bool close(void) = 0;

	//Fetch result
	virtual bool result(bool &success, uint32_t &status_code, uint64_t &file_size, uint64_t &time_stamp, std::wstring &content_type, std::wstring &content_encd) = 0;

	//Read payload
	virtual bool read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag) = 0;

	//Error message
	std::wstring get_error_text() const
	{
		return m_error_text.get();
	}

protected:
	//WinINet initialization
	bool wininet_init(void);
	bool wininet_exit(void);

	//Status callback
	static void __stdcall status_callback(void *hInternet, uintptr_t dwContext, uint32_t dwInternetStatus, void *lpvStatusInformation, uint32_t dwStatusInformationLength);
	virtual void update_status(const uint32_t &status, const uintptr_t &information);

	//Status messages
	void set_error_text(const std::wstring &text = std::wstring());
	void emit_message(const std::wstring message);

	//Utilities
	bool close_handle(void *&handle);
	bool set_inet_options(void *const request, const uint32_t &option, const uint32_t &value);
	bool get_inet_options(void *const request, const uint32_t &option, uint32_t &value);
	std::wstring status_str(const uintptr_t &info);

	//Const
	const Sync::Signal &m_user_aborted;
	const bool m_disable_proxy;
	const bool m_verbose;
	const std::wstring m_agent_str;
	const double m_timeout_con;
	const double m_timeout_rcv;
	const uint32_t m_connect_retry;

	//Thread-safety
	Sync::Mutex m_mutex;

	//Handle
	void *m_hInternet;

private:
	//Listener support
	Sync::Interlocked<std::set<AbstractListener*>> m_listeners;
	Sync::Interlocked<std::wstring> m_error_text;
};
