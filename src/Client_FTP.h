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

#include "Client_Abstract.h"

class FtpClient : public AbstractClient
{
public:
	//Constructor & destructor
	FtpClient(const bool &disable_proxy = false, const std::wstring &userAgentStr = std::wstring(), const bool &no_redir = false, const bool &insecure = false, const double &timeout_con = -1.0, const double &timeout_rcv = -1.0, const uint32_t &connect_retry = 3, const bool &verbose = false);
	virtual ~FtpClient(void);

	//Connection handling
	virtual bool open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer);
	virtual bool close(void);

	//Fetch result
	virtual bool result(bool &success, uint32_t &status_code, uint64_t &file_size, std::wstring &content_type, std::wstring &content_encd);

	//Read payload
	virtual bool read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag);
};
