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

#include "Utils.h"

class URL
{
public:
	URL(void);
	URL(const wchar_t *const url);
	~URL(void);

	//Getter
	inline const std::wstring &getScheme   (void) const { return m_strScheme;    }
	inline const std::wstring &getHostName (void) const { return m_strHostName;  }
	inline const std::wstring &getUserName (void) const { return m_strUserName;  }
	inline const std::wstring &getPassword (void) const { return m_strPassword;  }
	inline const std::wstring &getUrlPath  (void) const { return m_strUrlPath;   }
	inline const std::wstring &getExtraInfo(void) const { return m_strExtraInfo; }
	inline const uint16_t     &getPort     (void) const { return m_uiPortNumber; }

	//Setter
	inline void setScheme   (const wchar_t *const scheme)    { m_strScheme    = trim(std::wstring(scheme));    }
	inline void setHostName (const wchar_t *const hostName)  { m_strHostName  = trim(std::wstring(hostName));  }
	inline void setUserName (const wchar_t *const userName)  { m_strUserName  = trim(std::wstring(userName));  }
	inline void setPassword (const wchar_t *const password)  { m_strPassword  = trim(std::wstring(password));  }
	inline void setUrlPath  (const wchar_t *const urlPath)   { m_strUrlPath   = trim(std::wstring(urlPath));   }
	inline void setExtraInfo(const wchar_t *const extraInfo) { m_strExtraInfo = trim(std::wstring(extraInfo)); }
	inline void setPort     (const uint16_t &port)           { m_uiPortNumber = port;                          }

	//Complete?
	bool isComplete(void) const;

	//Create URL
	std::wstring toString(void) const;

private:
	std::wstring m_strScheme;
	std::wstring m_strHostName;
	std::wstring m_strUserName;
	std::wstring m_strPassword;
	std::wstring m_strUrlPath;
	std::wstring m_strExtraInfo;

	uint16_t m_uiPortNumber;
};

