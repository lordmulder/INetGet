///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
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

#include "Params.h"

//Internal
#include "URL.h"
#include "Utils.h"

//CRT
#include <stdexcept>
#include <iostream>
#include <limits>

//=============================================================================
// UTILITIES
//=============================================================================

static inline std::wstring argv_i(const wchar_t *const argv[], const int i)
{
	std::wstring temp(argv[i]);
	return trim(temp);
}

#define IS_OPTION(X) (option_key.compare(L##X) == 0)

#define ENSURE_VALUE() do \
{ \
	if(option_val.empty()) \
	{ \
		std::wcerr << L"ERROR: Required argument for option \"--" << option_key << "\" is missing!\n" << std::endl; \
		return false; \
	} \
} \
while(0)

#define ENSURE_NOVAL() do \
{ \
	if(!option_val.empty()) \
	{ \
		std::wcerr << L"ERROR: Excess argument \"" << option_val << "\" for option \"--" << option_key << "\" encountered!\n" << std::endl; \
		return false; \
	} \
} \
while(0)

#define PARSE_ENUM(X,Y) do \
{ \
	static const wchar_t *const name = L#Y; \
	if(_wcsicmp(value.c_str(), &name[(X)]) == 0) \
	{ \
		return (Y); \
	} \
} \
while(0)

#define PARSE_UINT32(X) do \
{ \
	try \
	{ \
		(X) = (_wcsicmp(option_val.c_str(), L"infinite") == 0) ? UINT32_MAX : std::stoul(option_val); \
	} \
	catch(std::exception&) \
	{ \
		std::wcerr << L"ERROR: Numeric value \"" << option_val << "\" could not be parsed!\n" << std::endl; \
		return false; \
	} \
} \
while(0)

#define PARSE_DOUBLE(X) do \
{ \
	try \
	{ \
		(X) = (_wcsicmp(option_val.c_str(), L"infinite") == 0) ? DBL_MAX : std::stod(option_val); \
	} \
	catch(std::exception&) \
	{ \
		std::wcerr << L"ERROR: Numeric value \"" << option_val << "\" could not be parsed!\n" << std::endl; \
		return false; \
	} \
} \
while(0)

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

Params::Params(void)
:
	m_iHttpVerb(HTTP_GET),
	m_bShowHelp(false),
	m_bDisableProxy(false),
	m_bDisableRedir(false),
	m_bInsecure(false),
	m_bEnableAlert(false),
	m_bVerboseMode(false),
	m_dTimeoutCon(std::numeric_limits<double>::quiet_NaN()),
	m_dTimeoutRcv(std::numeric_limits<double>::quiet_NaN()),
	m_uRetryCount(3)
{
}

Params::~Params()
{
}

//=============================================================================
// PARSE PARAMETERS
//=============================================================================

bool Params::initialize(const int argc, const wchar_t *const argv[])
{
	const std::wstring marker(L"--");

	bool stopFlag = false;
	size_t argCounter = 0;

	for(int i = 1; i < argc; i++)
	{
		const std::wstring current = argv_i(argv, i);
		if(!current.empty())
		{
			if((!stopFlag) && (current.find(marker) == 0))
			{
				if(current.length() > marker.length())
				{
					std::wstring option(current.substr(2, std::wstring::npos));
					if(!processOption(trim(option)))
					{
						return false;
					}
				}
				else
				{
					stopFlag = true;
				}
			}
			else
			{
				if(!(stopFlag = processParamN(argCounter++, current)))
				{
					return false;
				}
			}
		}
	}

	if((argCounter < 2) && (argc >= 2))
	{
		if((!_wcsicmp(argv[1], L"-h")) || (!_wcsicmp(argv[1], L"-?")) || (!_wcsicmp(argv[1], L"/?")))
		{
			m_bShowHelp = true;
		}
	}
		
	return validate();
}

bool Params::validate(void)
{
	if((m_strSource.empty() || m_strOutput.empty()) && (!m_bShowHelp))
	{
		std::wcerr << L"ERROR: Required parameter is missing!\n" << std::endl;
		return false;
	}

	if((!m_strReferrer.empty()) && (!URL(m_strReferrer).isComplete()))
	{
		std::wcerr << L"ERROR: The specified referrer address is invalid!\n" << std::endl;
		return false;
	}

	if(m_bInsecure)
	{
		std::wcerr << L"WARNING: Using insecure HTTPS mode, certificates will *not* be checked!\n";
	}

	if((!m_strPostData.empty()) && (m_iHttpVerb != HTTP_POST) && (m_iHttpVerb != HTTP_PUT))
	{
		std::wcerr << L"WARNING: Sending 'x-www-form-urlencoded' data, but HTTP verb is not POST/PUT!\n";
		std::wcerr << L"         You probably want to add the \"--verb=post\" or \"--verb=put\" argument.\n" << std::endl;
	}

	return true;
}

bool Params::processParamN(const size_t n, const std::wstring &param)
{
	switch(n)
	{
	case 0:
		m_strSource = param;
		return true;
	case 1:
		m_strOutput = param;
		return true;
	default:
		std::wcerr << L"ERROR: Excess argument \"" << param << L"\" encountered!\n" << std::endl;
		return false;
	}
}

bool Params::processOption(const std::wstring &option)
{
	std::wstring option_key, option_val;

	const size_t delim_pos = option.find_first_of(L'=');
	if(delim_pos != std::wstring::npos)
	{
		if((delim_pos == 0) || (delim_pos >= option.length() - 1))
		{
			std::wcerr << L"ERROR: Format of option \"--" << option << "\" is invalid!\n" << std::endl;
			return false;
		}
		option_key = option.substr(0, delim_pos);
		option_val = option.substr(delim_pos + 1, std::wstring::npos);
	}
	else
	{
		option_key = option;
	}

	return processOption(option_key, option_val);
}

bool Params::processOption(const std::wstring &option_key, const std::wstring &option_val)
{
	if(IS_OPTION("help"))
	{
		ENSURE_NOVAL();
		return (m_bShowHelp = true);
	}
	else if(IS_OPTION("verb"))
	{
		ENSURE_VALUE();
		return (HTTP_UNDEF != (m_iHttpVerb = parseHttpVerb(option_val)));
	}
	else if(IS_OPTION("data"))
	{
		ENSURE_VALUE();
		m_strPostData = option_val;
		return true;
	}
	else if(IS_OPTION("no-proxy"))
	{
		ENSURE_NOVAL();
		return (m_bDisableProxy = true);
	}
	else if(IS_OPTION("agent"))
	{
		ENSURE_VALUE();
		m_strUserAgent = option_val;
		return true;
	}
	else if(IS_OPTION("no-redir"))
	{
		ENSURE_NOVAL();
		return (m_bDisableRedir = true);
	}
	else if(IS_OPTION("insecure"))
	{
		ENSURE_NOVAL();
		return (m_bInsecure = true);
	}
	else if(IS_OPTION("refer"))
	{
		ENSURE_VALUE();
		m_strReferrer = option_val;
		return true;
	}
	else if(IS_OPTION("notify"))
	{
		ENSURE_NOVAL();
		return (m_bEnableAlert = true);
	}
	else if(IS_OPTION("time-cn"))
	{
		ENSURE_VALUE();
		PARSE_DOUBLE(m_dTimeoutCon);
		return true;
	}
	else if(IS_OPTION("time-rc"))
	{
		ENSURE_VALUE();
		PARSE_DOUBLE(m_dTimeoutRcv);
		return true;
	}
	else if(IS_OPTION("timeout"))
	{
		ENSURE_VALUE();
		PARSE_DOUBLE(m_dTimeoutCon);
		PARSE_DOUBLE(m_dTimeoutRcv);
		return true;
	}
	else if(IS_OPTION("retry"))
	{
		ENSURE_VALUE();
		PARSE_UINT32(m_uRetryCount);
		return true;
	}
	else if(IS_OPTION("no-retry"))
	{
		ENSURE_NOVAL();
		m_uRetryCount = 0;
		return true;
	}
	else if(IS_OPTION("verbose"))
	{
		ENSURE_NOVAL();
		return (m_bVerboseMode = true);
	}

	std::wcerr << L"ERROR: Unknown option \"--" << option_key << "\" encountered!\n" << std::endl;
	return false;
}

http_verb_t Params::parseHttpVerb(const std::wstring &value)
{
	PARSE_ENUM(5, HTTP_GET);
	PARSE_ENUM(5, HTTP_POST);
	PARSE_ENUM(5, HTTP_PUT);
	PARSE_ENUM(5, HTTP_DELETE);
	PARSE_ENUM(5, HTTP_HEAD);

	std::wcerr << L"ERROR: Unknown HTTP method \"" << value << "\" encountered!\n" << std::endl;
	return HTTP_UNDEF;
}
