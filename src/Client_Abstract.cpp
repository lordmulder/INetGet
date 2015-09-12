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

#include "Client_Abstract.h"

//Internal
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <WinInet.h>

//CRT
#include <iostream>

//Agent String
static const wchar_t *const USER_AGENT = L"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9) Gecko/2008062901 IceWeasel/3.0";

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

AbstractClient::AbstractClient()
:
	m_hInternet(NULL)
{
}

AbstractClient::~AbstractClient()
{
	exit_client();
}

//=============================================================================
// INITIALIZE OR EXIT CLIENT
//=============================================================================

bool AbstractClient::init_client(void)
{
	if(m_hInternet == NULL)
	{
		m_hInternet = InternetOpen(USER_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(m_hInternet == NULL)
		{
			const DWORD error_code = GetLastError();
			std::wcerr << "InternetOpen() has failed: " << error_string(error_code) << L'\n' << std::endl;
		}
	}
	return (m_hInternet != NULL);
}

bool AbstractClient::exit_client(void)
{
	BOOL success = TRUE;
	if(m_hInternet != NULL)
	{
		BOOL success = InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
	return (success == TRUE);
}
