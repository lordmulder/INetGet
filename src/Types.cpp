///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2018 LoRd_MuldeR <MuldeR2@GMX.de>
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

#include "Types.h"

#include <cstdlib>

const http_status_t STATUS_CODES[] =
{
	{ 100, L"Continue" },
	{ 101, L"Switching Protocols" },
	{ 102, L"Processing" },
	{ 200, L"OK" },
	{ 201, L"Created" },
	{ 202, L"Accepted" },
	{ 203, L"Non-Authoritative Information" },
	{ 204, L"No Content" },
	{ 205, L"Reset Content" },
	{ 206, L"Partial Content" },
	{ 207, L"Multi-Status" },
	{ 208, L"Already Reported" },
	{ 226, L"IM Used" },
	{ 300, L"Multiple Choices" },
	{ 301, L"Moved Permanently" },
	{ 302, L"Found" },
	{ 303, L"See Other" },
	{ 304, L"Not Modified" },
	{ 305, L"Use Proxy" },
	{ 306, L"Switch Proxy" },
	{ 307, L"Temporary Redirect" },
	{ 308, L"Permanent Redirect" },
	{ 400, L"Bad Request" },
	{ 401, L"Unauthorized" },
	{ 402, L"Payment Required" },
	{ 403, L"Forbidden" },
	{ 404, L"Not Found" },
	{ 405, L"Method Not Allowed" },
	{ 406, L"Not Acceptable" },
	{ 407, L"Proxy Authentication Required" },
	{ 408, L"Request Time-out" },
	{ 409, L"Conflict" },
	{ 410, L"Gone" },
	{ 411, L"Length Required" },
	{ 412, L"Precondition Failed" },
	{ 413, L"Request Entity Too Large" },
	{ 414, L"Request-URL Too Long" },
	{ 415, L"Unsupported Media Type" },
	{ 416, L"Requested range not satisfiable" },
	{ 417, L"Expectation Failed" },
	{ 418, L"I’m a teapot" },
	{ 420, L"Policy Not Fulfilled" },
	{ 421, L"There are too many connections from your internet address" },
	{ 422, L"Unprocessable Entity" },
	{ 423, L"Locked" },
	{ 424, L"Failed Dependency" },
	{ 425, L"Unordered Collection" },
	{ 426, L"Upgrade Required" },
	{ 428, L"Precondition Required" },
	{ 429, L"Too Many Requests" },
	{ 431, L"Request Header Fields Too Large" },
	{ 444, L"No Response" },
	{ 449, L"The request should be retried after doing the appropriate action" },
	{ 451, L"Unavailable For Legal Reasons" },
	{ 500, L"Internal Server Error" },
	{ 501, L"Not Implemented" },
	{ 502, L"Bad Gateway" },
	{ 503, L"Service Unavailable" },
	{ 504, L"Gateway Time-out" },
	{ 505, L"HTTP Version not supported" },
	{ 506, L"Variant Also Negotiates" },
	{ 507, L"Insufficient Storage" },
	{ 508, L"Loop Detected" },
	{ 509, L"Bandwidth Limit Exceeded" },
	{ 510, L"Not Extended" },
	{ 0, NULL }
};
