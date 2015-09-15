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

#include <string>
#include <stdint.h>

class URL
{
public:
	URL(const URL &url);
	URL(const std::wstring &url);
	~URL(void);

	//Getter
	inline const int          &getScheme   (void) const { return m_iSchemeId;    }
	inline const std::wstring &getHostName (void) const { return m_strHostName;  }
	inline const std::wstring &getUserName (void) const { return m_strUserName;  }
	inline const std::wstring &getPassword (void) const { return m_strPassword;  }
	inline const std::wstring &getUrlPath  (void) const { return m_strUrlPath;   }
	inline const std::wstring &getExtraInfo(void) const { return m_strExtraInfo; }
	inline const uint16_t     &getPortNo   (void) const { return m_uiPortNumber; }

	//Complete?
	bool isComplete(void) const;

private:
	std::wstring m_strScheme;
	std::wstring m_strHostName;
	std::wstring m_strUserName;
	std::wstring m_strPassword;
	std::wstring m_strUrlPath;
	std::wstring m_strExtraInfo;

	int m_iSchemeId;
	uint16_t m_uiPortNumber;
};

