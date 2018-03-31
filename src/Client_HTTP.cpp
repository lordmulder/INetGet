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

#include "Client_HTTP.h"

//Internal
#include "URL.h"
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinINet.h>

//CRT
#include <stdint.h>
#include <stdexcept>
#include <sstream>
#include <cmath>

//Helper functions
static const wchar_t *CSTR(const std::wstring &str) { return str.empty() ? NULL : str.c_str(); }
static const char    *CSTR(const std::string  &str) { return str.empty() ? NULL : str.c_str(); }

//Const
static const wchar_t *const HTTP_VER_11      = L"HTTP/1.1";
static const wchar_t *const ACCEPTED_TYPES[] = { L"*/*", NULL };
static const wchar_t *const TYPE_FORM_DATA   = L"Content-Type: application/x-www-form-urlencoded";
static const wchar_t *const MODIFIED_SINCE   = L"If-Modified-Since: ";
static const wchar_t *const RANGE_BYTES      = L"Range: bytes=";
//Macros
#define OPTIONAL_FLAG(X,Y,Z) do \
{ \
	if((Y)) { (X) |= (Z); } \
} \
while(0)

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

HttpClient::HttpClient(const Sync::Signal &user_aborted, const bool &disableProxy, const std::wstring &userAgentStr, const bool &no_redir, uint64_t range_start, uint64_t range_end, const bool &insecure, const bool &force_crl, const double &timeout_con, const double &timeout_rcv, const uint32_t &connect_retry, const bool &verbose)
:
	AbstractClient(user_aborted, disableProxy, userAgentStr, timeout_con, timeout_rcv, connect_retry, verbose),
	m_disable_redir(no_redir),
	m_range_start(range_start),
	m_range_end(range_end),
	m_insecure_tls(insecure),
	m_force_crl(force_crl),
	m_hConnection(NULL),
	m_hRequest(NULL),
	m_current_status(UINT32_MAX)
{
	if(m_insecure_tls && m_force_crl)
	{
		throw std::runtime_error("Flags 'insecure' and 'force_crl' are mutually exclusive!");
	}
}

HttpClient::~HttpClient(void)
{
}

//=============================================================================
// CONNECTION HANDLING
//=============================================================================

bool HttpClient::open(const http_verb_t &verb, const URL &url, const std::string &post_data, const std::wstring &referrer, const uint64_t &timestamp)
{
	Sync::Locker locker(m_mutex);
	if(!wininet_init())
	{
		return false; /*WinINet failed to initialize*/
	}

	//Close the existing connection, just to be sure
	if(!close())
	{
		set_error_text(std::wstring(L"ERROR: Failed to close the existing connection!"));
		return false;
	}

	//Print URL details
	const bool use_tls = (url.getScheme() == INTERNET_SCHEME_HTTPS);
	if(m_verbose)
	{
		std::wostringstream url_str;
		url_str << (use_tls ? L"HTTPS" : L"HTTP") <<  L'|' << url.getHostName() << L'|' << url.getUserName() << L'|' << url.getPassword() << L'|' << url.getPortNo() << L'|' << url.getUrlPath() << url.getExtraInfo();
		emit_message(std::wstring(L"RQST_URL: \"") + url_str.str() + L'"');
		emit_message(std::wstring(L"REFERRER: \"") + referrer + L'"');
		emit_message(std::wstring(L"POST_DAT: \"") + Utils::utf8_to_wide_str(post_data.c_str()) + L'"');
	}

	//Reset status
	m_current_status = UINT32_MAX;

	//Create connection
	if(!connect(url.getHostName(), url.getPortNo(), url.getUserName(), url.getPassword()))
	{
		return false; /*the connection could not be created*/
	}

	//Create HTTP request and send!
	if(!create_request(use_tls, verb, url.getUrlPath(), url.getExtraInfo(), post_data, referrer, timestamp))
	{
		return false; /*the request could not be created or sent*/
	}

	//Sucess
	set_error_text();
	emit_message(std::wstring(L"Response received."));
	return true;
}

bool HttpClient::close(void)
{
	Sync::Locker locker(m_mutex);
	bool success = true;

	//Close the request, if it currently exists
	if(!close_handle(m_hRequest))
	{
		success = false;
	}

	//Close connection, if it is currently open
	if(!close_handle(m_hConnection))
	{
		success = false;
	}

	set_error_text();
	return success;
}

//=============================================================================
// QUERY RESULT
//=============================================================================

bool HttpClient::result(bool &success, uint32_t &status_code, uint64_t &file_size, uint64_t &time_stamp, std::wstring &content_type, std::wstring &content_encd)
{
	success = false;
	status_code = 0;
	file_size = SIZE_UNKNOWN;
	time_stamp = TIME_UNKNOWN;
	content_type.clear();
	content_encd.clear();

	Sync::Locker locker(m_mutex);

	if(m_hRequest == NULL)
	{
		set_error_text(std::wstring(L"INTERNAL ERROR: There currently is no active request!"));
		return false; /*request not created yet*/
	}

	if(!get_header_int(m_hRequest, HTTP_QUERY_STATUS_CODE, status_code))
	{
		return false; /*couldn't get status code*/
	}

	get_header_str(m_hRequest, HTTP_QUERY_CONTENT_TYPE,     content_type);
	get_header_str(m_hRequest, HTTP_QUERY_CONTENT_ENCODING, content_encd);

	std::wstring content_length;
	if(get_header_str(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, content_length))
	{
		file_size = parse_file_size(content_length);
	}

	std::wstring last_modified;
	if(get_header_str(m_hRequest, HTTP_QUERY_LAST_MODIFIED, last_modified))
	{
		time_stamp = Utils::parse_timestamp(last_modified);
	}

	set_error_text();
	success = ((status_code >= 200) && (status_code < 300));
	return true;
}

//=============================================================================
// READ PAYLOAD
//=============================================================================

bool HttpClient::read_data(uint8_t *out_buff, const uint32_t &buff_size, size_t &bytes_read, bool &eof_flag)
{
	Sync::Locker locker(m_mutex);

	if(m_hRequest == NULL)
	{
		set_error_text(std::wstring(L"INTERNAL ERROR: There currently is no active request!"));
		return false; /*request not created yet*/
	}

	DWORD temp;
	if(!InternetReadFile(m_hRequest, out_buff, buff_size, &temp))
	{
		const DWORD error_code = GetLastError();
		set_error_text(std::wstring(L"An error occurred while receiving data from the server:\n").append(Utils::win_error_string(error_code)));
		return false;
	}

	set_error_text();
	eof_flag = ((bytes_read = temp) < 1);
	return true;
}

//=============================================================================
// INTERNAL FUNCTIONS
//=============================================================================

bool HttpClient::connect(const std::wstring &hostName, const uint16_t &portNo, const std::wstring &userName, const std::wstring &password)
{
	//Try to open the new connection
	m_hConnection = InternetConnect(m_hInternet, CSTR(hostName), portNo, CSTR(userName), CSTR(password), INTERNET_SERVICE_HTTP, 0, reinterpret_cast<intptr_t>(this));
	if(m_hConnection == NULL)
	{
		const DWORD error_code = GetLastError();
		set_error_text(std::wstring(L"InternetConnect() function has failed:\n").append(Utils::win_error_string(error_code)));
		return false;
	}

	//Install the callback handler (only in verbose mode)
	if(InternetSetStatusCallback(m_hConnection, (INTERNET_STATUS_CALLBACK)(&status_callback)) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		const DWORD error_code = GetLastError();
		set_error_text(std::wstring(L"InternetSetStatusCallback() function has failed:\n").append(Utils::win_error_string(error_code)));
		return false;
	}

	return (!m_user_aborted.get());
}

bool HttpClient::create_request(const bool &use_tls, const http_verb_t &verb, const std::wstring &path, const std::wstring &query, const std::string &post_data, const std::wstring &referrer, const uint64_t &timestamp)
{
	//Setup request flags
	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	OPTIONAL_FLAG(flags, use_tls,         INTERNET_FLAG_SECURE);
	OPTIONAL_FLAG(flags, m_disable_redir, INTERNET_FLAG_NO_AUTO_REDIRECT);
	OPTIONAL_FLAG(flags, m_insecure_tls,  INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP);
	OPTIONAL_FLAG(flags, m_disable_proxy, INTERNET_FLAG_PRAGMA_NOCACHE);

	//Try to create the HTTP request
	m_hRequest = HttpOpenRequestW(m_hConnection, http_verb_str(verb), CSTR(path + query), HTTP_VER_11, CSTR(referrer), (LPCWSTR*)ACCEPTED_TYPES, flags, intptr_t(this));
	if(m_hRequest == NULL)
	{
		const DWORD error_code = GetLastError();
		set_error_text(std::wstring(L"HttpOpenRequest() function has failed:\n").append(Utils::win_error_string(error_code)));
		return false;
	}

	//Update the security flags, if required
	static const DWORD insecure_flags = SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WRONG_USAGE;
	if(!update_security_opts(m_hRequest, insecure_flags, m_insecure_tls))
	{
		return false; /*updating the flags failed!*/
	}

	//Prepare headers
	std::wostringstream headers;
	if(post_data.length() > 0)
	{
		headers << TYPE_FORM_DATA << std::endl;
	}
	if(timestamp > TIME_UNKNOWN)
	{
		headers << MODIFIED_SINCE << Utils::timestamp_to_str(timestamp) << std::endl;
	}
	if((m_range_start > 0U) || (m_range_end != UINT64_MAX))
	{
		if(m_range_end != UINT64_MAX)
		{
			headers << RANGE_BYTES << m_range_start << '-' << m_range_end << std::endl;
		}
		else
		{
			headers << RANGE_BYTES << m_range_start << '-' << std::endl;
		}
	}

	//Setup retry point
	uint32_t retry_counter = 0;
	bool retry_flag = m_insecure_tls || m_force_crl;
	label_retry_create_request:

	//Try to actually send the HTTP request
	BOOL success = HttpSendRequest(m_hRequest, CSTR(headers.str()), DWORD(-1L), ((LPVOID)CSTR(post_data)), DWORD(post_data.length()));
	if(success != TRUE)
	{
		const DWORD error_code = GetLastError();
		if(m_user_aborted.get())
		{
			return false; /*aborted by user*/
		}
		if((error_code == ERROR_INTERNET_SEC_CERT_REV_FAILED) && (!retry_flag))
		{
			if(retry_flag = update_security_opts(m_hRequest, SECURITY_FLAG_IGNORE_REVOCATION, true))
			{
				emit_message(std::wstring(L"Failed to check for revocation, retrying with revocation checks disabled!"));
				goto label_retry_create_request;
			}
		}
		else if(((error_code == ERROR_INTERNET_CANNOT_CONNECT) || (error_code == ERROR_INTERNET_TIMEOUT)) && (retry_counter++ < m_connect_retry))
		{
			std::wostringstream retry_info;
			retry_info << L'[' << retry_counter << L'/' << m_connect_retry << L']';
			emit_message(std::wstring(L"Connection has failed. Retrying! ").append(retry_info.str()));
			goto label_retry_create_request;
		}
		set_error_text(std::wstring(L"Failed to connect to the server:\n").append(Utils::win_error_string(error_code)));
		return false;
	}
	
	return (!m_user_aborted.get());
}

//=============================================================================
// STATUS HANDLER
//=============================================================================

void HttpClient::update_status(const uint32_t &status, const uintptr_t &info)
{
	if(m_user_aborted.get())
	{
		return; /*aborted by user*/
	}

	AbstractClient::update_status(status, info);
	if(m_current_status != status)
	{
		std::wostringstream status_info;
		switch(status)
		{
			case INTERNET_STATUS_DETECTING_PROXY:      status_info << "Detecting proxy server..."                       ; break;
			case INTERNET_STATUS_RESOLVING_NAME:       status_info << "Resolving host name..."                          ; break;
			case INTERNET_STATUS_NAME_RESOLVED:        status_info << "Server address resolved to: " << status_str(info); break;
			case INTERNET_STATUS_CONNECTING_TO_SERVER: status_info << "Connecting to server..."                         ; break;
			case INTERNET_STATUS_SENDING_REQUEST:      status_info << "Sending request to server..."                    ; break;
			case INTERNET_STATUS_REDIRECT:             status_info << "Redirecting: " << status_str(info)               ; break;
			case INTERNET_STATUS_RECEIVING_RESPONSE:   status_info << "Request sent, awaiting response..."              ; break;
			default: return; /*unknown code*/
		}
		emit_message(status_info.str());
		m_current_status = status;
	}
}

//=============================================================================
// UTILITIES
//=============================================================================

#define CHECK_HTTP_VERB(X) do \
{ \
	if((X) == verb) \
	{ \
		static const wchar_t *const name = L#X; \
		return &name[5]; \
	} \
} \
while(0)

const wchar_t *HttpClient::http_verb_str(const http_verb_t &verb)
{
	CHECK_HTTP_VERB(HTTP_GET);
	CHECK_HTTP_VERB(HTTP_POST);
	CHECK_HTTP_VERB(HTTP_PUT);
	CHECK_HTTP_VERB(HTTP_DELETE);
	CHECK_HTTP_VERB(HTTP_HEAD);

	throw new std::runtime_error("Invalid verb specified!");
}

bool HttpClient::update_security_opts(void *const request, const uint32_t &new_flags, const bool &enable)
{
	uint32_t security_flags = 0;
	if(get_inet_options(request, INTERNET_OPTION_SECURITY_FLAGS, security_flags))
	{
		security_flags = enable ? (security_flags | new_flags) : (security_flags & (~new_flags));
		return set_inet_options(request, INTERNET_OPTION_SECURITY_FLAGS, security_flags);
	}
	return false;
}

bool HttpClient::get_header_int(void *const request, const uint32_t type, uint32_t &value)
{
	DWORD result;
	DWORD resultSize = sizeof(DWORD);

	if(HttpQueryInfo(request, type | HTTP_QUERY_FLAG_NUMBER, &result, &resultSize, 0) == TRUE)
	{
		value = result;
		return true;
	}

	const DWORD error_code = GetLastError();
	if(error_code != ERROR_HTTP_HEADER_NOT_FOUND)
	{
		set_error_text(std::wstring(L"HttpQueryInfo() has failed:\n").append(Utils::win_error_string(error_code)));
	}

	return false;
}

bool HttpClient::get_header_str(void *const request, const uint32_t type, std::wstring &value)
{
	static const size_t BUFF_SIZE = 2048;
	wchar_t result[BUFF_SIZE];
	DWORD resultSize = BUFF_SIZE * sizeof(wchar_t);

	if(HttpQueryInfo(request, type, &result, &resultSize, 0) == TRUE)
	{
		std::wstring temp(result);
		value = Utils::trim(temp);
		return (!value.empty());
	}

	const DWORD error_code = GetLastError();
	if(error_code != ERROR_HTTP_HEADER_NOT_FOUND)
	{
		set_error_text(std::wstring(L"HttpQueryInfo() has failed:\n").append(Utils::win_error_string(error_code)));
	}

	value.clear();
	return false;
}

uint64_t HttpClient::parse_file_size(const std::wstring &str)
{
	if(str.empty())
	{
		return SIZE_UNKNOWN; /*string is empty*/
	}

	try
	{
		const uint64_t result = std::stoull(str);
		return (result > 0ui64) ? result : SIZE_UNKNOWN;
	}
	catch(std::exception&)
	{
		return SIZE_UNKNOWN; /*parsing error*/
	}
}


