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
#include <string>

class AbstractClient
{
public:
	static const uint32_t SIZE_UNKNOWN = UINT32_MAX;

	AbstractClient(const bool &verbose);
	~AbstractClient(void);

	bool client_init(const bool &disableProxy = false, const std::wstring &userAgent = std::wstring());
	bool client_exit(void);

	virtual bool connection_init(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &useName, const std::wstring &password) = 0;
	virtual bool connection_exit(void);

	virtual bool request_init(const std::wstring &verb, const std::wstring &path, const bool &secure) = 0;
	virtual bool request_exit(void);

	virtual bool query_result(bool &success, uint32_t &status_code, uint32_t &file_size, std::wstring &content_type) = 0;

protected:
	bool connection_init(const uint32_t &serviceId, const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password);

	static void __stdcall callback_handler(void *hInternet, uintptr_t dwContext, uint32_t dwInternetStatus, void *lpvStatusInformation, uint32_t dwStatusInformationLength);
	virtual void update_status(const uint32_t &dwInternetStatus, const uint32_t &lpvStatusInformation);

	const bool m_verbose;

	void *m_hInternet;
	void *m_hConnection;
	void *m_hRequest;
};
