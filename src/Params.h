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

class Params
{
public:
	Params(void);
	~Params(void);

	bool initialize(const int argc, const wchar_t *const argv[]);

	//Getter
	inline const std::wstring &getSource(  void) const { return m_strSource; }
	inline const std::wstring &getOutput  (void) const { return m_strOutput; }
	inline const bool         &getShowHelp(void) const { return m_bShowHelp; }

private:
	bool processParamN(const size_t n, const std::wstring &param);
	bool processOption(const std::wstring &option);

	std::wstring m_strSource;
	std::wstring m_strOutput;

	bool m_bShowHelp;
};

