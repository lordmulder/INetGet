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

#include "Client_Abstract.h"

class HttpClient : public AbstractClient
{
public:
	HttpClient(const bool &verbose);
	~HttpClient(void);

	virtual bool connection_init(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password);
	virtual bool request_init(const std::wstring &verb, const std::wstring &path, const bool &secure);
	virtual bool query_result(bool &success, uint32_t &status_code, uint32_t &file_size, std::wstring &content_type);
};

