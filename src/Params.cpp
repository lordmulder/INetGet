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

#include "Params.h"

//Internal
#include "Utils.h"

//CRT
#include <stdexcept>
#include <iostream>

//=============================================================================
// UTILITIES
//=============================================================================

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
		std::wcerr << L"ERROR: Excess argument for option \"--" << option_key << "\" encountered!\n" << std::endl; \
		return false; \
	} \
} \
while(0)

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

Params::Params(void)
:
	m_bShowHelp(false),
	m_bDisableProxy(false)
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
		const std::wstring current = trim(std::wstring(argv[i]));
		if(!current.empty())
		{
			if((!stopFlag) && (current.find(marker) == 0))
			{
				if(current.length() > marker.length())
				{
					if(!processOption(trim(current.substr(2, std::wstring::npos))))
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

	if(!(m_bShowHelp || (argCounter >= 2)))
	{
		std::wcerr << L"ERROR: Required parameter is missing!\n" << std::endl;
		return false;
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
		std::wcerr << L"ERROR: Excess parameter \"" << param << L"\" encountered!\n" << std::endl;
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

	std::wcerr << L"ERROR: Unknown option \"--" << option_key << "\" encountered!\n" << std::endl;
	return false;
}
