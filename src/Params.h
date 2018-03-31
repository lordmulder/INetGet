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

#pragma once

#include <string>
#include "Types.h"

class Params
{
public:
	Params(void);
	~Params(void);

	bool parse_cli_args(const int argc, const wchar_t *const argv[]);
	bool load_conf_file(const std::wstring &config_file);

	//Getter
	inline const std::wstring &getSource       (void) const { return m_strSource;     }
	inline const std::wstring &getOutput       (void) const { return m_strOutput;     }
	inline const http_verb_t  &getHttpVerb     (void) const { return m_iHttpVerb;     }
	inline const std::wstring &getPostData     (void) const { return m_strPostData;   }
	inline const bool         &getShowHelp     (void) const { return m_bShowHelp;     }
	inline const bool         &getDisableProxy (void) const { return m_bDisableProxy; }
	inline const std::wstring &getUserAgent    (void) const { return m_strUserAgent;  }
	inline const bool         &getDisableRedir (void) const { return m_bDisableRedir; }
	inline const uint64_t     &getRangeStart   (void) const { return m_uRangeStart;   }
	inline const uint64_t     &getRangeEnd     (void) const { return m_uRangeEnd;     }
	inline const bool         &getInsecure     (void) const { return m_bInsecure;     }
	inline const std::wstring &getReferrer     (void) const { return m_strReferrer;   }
	inline const bool         &getEnableAlert  (void) const { return m_bEnableAlert;  }
	inline const double       &getTimeoutCon   (void) const { return m_dTimeoutCon;   }
	inline const double       &getTimeoutRcv   (void) const { return m_dTimeoutRcv;   }
	inline const uint32_t     &getRetryCount   (void) const { return m_uRetryCount;   }
	inline const bool         &getForceCrl     (void) const { return m_bForceCrl;     }
	inline const bool         &getSetTimestamp (void) const { return m_bSetTimestamp; }
	inline const bool         &getUpdateMode   (void) const { return m_bUpdateMode;   }
	inline const bool         &getKeepFailed   (void) const { return m_bKeepFailed;   }
	inline const bool         &getVerboseMode  (void) const { return m_bVerboseMode;  }

private:
	bool validate(const bool &is_final);
	bool load_conf_file(const std::wstring &config_file, const bool &recursive);

	bool processParamN(const size_t n, const std::wstring &param);
	bool processOption(const std::wstring &option);
	bool processOption(const std::wstring &option_key, const std::wstring &option_val);

	static http_verb_t parseHttpVerb(const std::wstring &value);

	std::wstring m_strSource;
	std::wstring m_strOutput;
	http_verb_t  m_iHttpVerb;
	std::wstring m_strPostData;
	bool         m_bShowHelp;
	bool         m_bDisableProxy;
	std::wstring m_strUserAgent;
	bool         m_bDisableRedir;
	uint64_t     m_uRangeStart;
	uint64_t     m_uRangeEnd;
	bool         m_bInsecure;
	std::wstring m_strReferrer;
	bool         m_bEnableAlert;
	double       m_dTimeoutCon;
	double       m_dTimeoutRcv;
	uint32_t     m_uRetryCount;
	bool         m_bForceCrl;
	bool         m_bSetTimestamp;
	bool         m_bUpdateMode;
	bool         m_bKeepFailed;
	bool         m_bVerboseMode;
};

