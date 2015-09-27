///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
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

#include "Sink_File.h"

//Internal
#include "Utils.h"

//Win32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//CRT
#include <cstdio>
#include <iostream>

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

FileSink::FileSink(const std::wstring &fileName, const uint64_t &timestamp, const bool &keepFailed)
:
	m_handle(NULL),
	m_timestamp(timestamp),
	m_fileName(fileName),
	m_keepFailed(keepFailed)
{
}

FileSink::~FileSink(void)
{
	close(false);
}

//=============================================================================
// OPEN / CLOSE
//=============================================================================

bool FileSink::open(void)
{
	//Close existign file, just to be sure
	close(false);

	//Try to open the file now
	FILE *hFile = NULL;
	if(_wfopen_s(&hFile, m_fileName.c_str(), L"wb") != 0)
	{
		const int error_code = errno;
		std::wcerr << L"The specified output file could not be opened for writing:\n" << Utils::crt_error_string(error_code) << L'\n' << std::endl;
		return false;
	}

	m_handle = uintptr_t(hFile);
	return true;
}

bool FileSink::close(const bool &success)
{
	bool okay = true;

	if(FILE *const hFile = (FILE*)m_handle)
	{
		if(success && (m_timestamp > 0))
		{
			fflush(hFile);
			Utils::set_file_time(_fileno(hFile), m_timestamp);
		}
	
		okay = (fclose(hFile) == 0);

		if((!success) && (!m_keepFailed))
		{
			_wremove(m_fileName.c_str());
		}
	}


	m_handle = NULL;
	return okay;
}

//=============================================================================
// WRITE
//=============================================================================

bool FileSink::write(uint8_t *const buffer, const size_t &count)
{
	if(FILE *const hFile = (FILE*)m_handle)
	{
		if(count > 0)
		{
			if(!ferror(hFile))
			{
				const size_t bytesWritten = fwrite(buffer, sizeof(uint8_t), count, hFile);
				if(bytesWritten != count)
				{
					const int error_code = errno;
					std::wcerr << L"\b\b\bfailed!\n\nAn I/O error occurred while trying to write to output file:\n" << Utils::crt_error_string(error_code) << L'\n' << std::endl;
					return false;
				}
				return true;
			}
			return false;
		}
		return true;
	}
	return false;
}