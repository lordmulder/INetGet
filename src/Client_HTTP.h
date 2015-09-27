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

class HttpClient : public AbstractClient
{
public:
	//Constructor & destructor
	HttpClient(const bool &disable_proxy = false, const std::wstring &userAgentStr = std::wstring(), const bool &no_redir = false, const bool &insecure = false, const bool &force_crl = false, const double &timeout_con = -1.0, const double &timeout_rcv = -1.0, const uint32_t &connect_retry = 3, const bool &verbose = false);
	virtual ~HttpClient(void);

	//Connection handling
	virtual bool open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer);
	virtual bool close(void);

	//Fetch result
	virtual bool result(bool &success, uint32_t &status_code, uint64_t &file_size, uint64_t &time_stamp, std::wstring &content_type, std::wstring &content_encd);

	//Read payload
	virtual bool read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag);

private:
	//Create connection/request
	bool connect(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password);
	bool create_request(const bool &use_tls, const http_verb_t &verb, const std::wstring &path, const std::wstring &query, const std::string &post_data, const std::wstring &referrer);

	//Status handler
	virtual void update_status(const uint32_t &status, const uintptr_t &information);

	//Utilities
	static const wchar_t *http_verb_str(const http_verb_t &verb);
	static bool update_security_opts(void *const request, const uint32_t &new_flags, const bool &enable);
	static bool get_header_int(void *const request, const uint32_t type, uint32_t &value);
	static bool get_header_str(void *const request, const uint32_t type, std::wstring &value);
	static uint64_t parse_file_size(const std::wstring &str);

	//Handles
	void *m_hConnection;
	void *m_hRequest;
	
	//Const
	const bool m_insecure_tls;
	const bool m_force_crl;
	const bool m_disable_redir;

	//Current status
	uint32_t m_current_status;
};

