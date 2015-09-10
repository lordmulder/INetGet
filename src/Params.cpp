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

#include <stdexcept>
#include <iostream>

#include "Utils.h"

Params::Params(const int argc, wchar_t *const argv[])
{
	const std::wstring marker(L"--");
	bool stopFlag = false; size_t counter = 0;

	for(int i = 1; i < argc; i++)
	{
		const std::wstring current = trim(std::wstring(argv[i]));
		if((!stopFlag) && (current.find(marker) == 0))
		{
			if(current.length() > marker.length())
			{
				processOption(trim(current.substr(2, std::wstring::npos)));
			}
			else
			{
				stopFlag = true;
			}
		}
		else
		{
			processParamN(counter++, current);
		}
	}
}

Params::~Params()
{
}

void Params::processParamN(const size_t n, const std::wstring &param)
{
	switch(n)
	{
	case 0:
		std::wcout << L"Param #1: \"" << param << '"' << std::endl;
		break;
	case 1:
		std::wcout << L"Param #2: \"" << param << '"' << std::endl;
		break;
	default:
		throw std::invalid_argument("Too many parameters!");
	}
}

void Params::processOption(const std::wstring &option)
{
	std::wcout << L"Option: \"" << option << '"' << std::endl;
}
