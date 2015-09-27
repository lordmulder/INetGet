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

#include "Types.h"

class URL;

#include <stdint.h>
#include <string>

class AbstractClient
{
public:
	static const uint64_t SIZE_UNKNOWN = UINT64_MAX;
	static const uint64_t TIME_UNKNOWN = 0;

	AbstractClient(const bool &disable_proxy = false, const std::wstring &userAgentStr = std::wstring(), const double &timeout_con = -1.0, const double &timeout_rcv = -1.0, const uint32_t &connect_retry = 3, const bool &verbose = false);
	virtual ~AbstractClient(void);

	//Connection handling
	virtual bool open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer) = 0;
	virtual bool close(void) = 0;

	//Fetch result
	virtual bool result(bool &success, uint32_t &status_code, uint64_t &file_size, uint64_t &time_stamp, std::wstring &content_type, std::wstring &content_encd) = 0;

	//Read payload
	virtual bool read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag) = 0;

protected:
	//WinINet initialization
	bool wininet_init(void);
	bool wininet_exit(void);

	//Status callback
	static void __stdcall status_callback(void *hInternet, uintptr_t dwContext, uint32_t dwInternetStatus, void *lpvStatusInformation, uint32_t dwStatusInformationLength);
	virtual void update_status(const uint32_t &status, const uintptr_t &information);

	//Utilities
	static bool close_handle(void *&handle);
	static bool set_inet_options(void *const request, const uint32_t &option, const uint32_t &value);
	static bool get_inet_options(void *const request, const uint32_t &option, uint32_t &value);
	static std::wstring status_str(const uintptr_t &info);

	//Const
	const bool m_disable_proxy;
	const bool m_verbose;
	const std::wstring m_userAgentStr;
	const double m_timeout_con;
	const double m_timeout_rcv;
	const uint32_t m_connect_retry;

	//Handle
	void *m_hInternet;
};
