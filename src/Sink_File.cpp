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

#include "Sink_File.h"

//Internal
#include "Utils.h"

//CRT
#include <cstdio>
#include <iostream>

//=============================================================================
// CONSTRUCTOR / DESTRUCTOR
//=============================================================================

FileSink::FileSink(const std::wstring &fileName)
:
	m_handle(NULL),
	m_fileName(fileName)
{
}

FileSink::~FileSink(void)
{
	close();
}

//=============================================================================
// OPEN / CLOSE
//=============================================================================

bool FileSink::open(void)
{
	//Close existign file, just to be sure
	close();

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

bool FileSink::close(void)
{
	bool success = true;

	if(FILE *const hFile = (FILE*)m_handle)
	{
		success = (fclose(hFile) == 0);
	}

	m_handle = NULL;
	return success;
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