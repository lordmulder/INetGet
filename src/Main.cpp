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

#include "URL.h"
#include "Utils.h"
#include "Params.h"

#include <iostream>
#include <stdexcept>

static void test(const wchar_t *const str)
{
	std::wcout << L'"' << trim(std::wstring(str)) << L"\" <= \"" << str << L'"' << std::endl;
}

int wmain(int argc, wchar_t* argv[])
{
	try
	{
		Params params(argc, argv);
	}
	catch(std::invalid_argument &e)
	{
		std::cerr << "Invalid arguments: " << e.what() << std::endl;
	}

	/*
	test(L"");
	test(L" ");
	test(L"  ");
	test(L"   ");
	
	std::wcout << std::endl;

	test(L"!");
	test(L"!  ");
	test(L" ! ");
	test(L"  !");

	std::wcout << std::endl;

	test(L"! ! !");
	test(L"! ! !      ");
	test(L"   ! ! !   ");
	test(L"      ! ! !");

		std::wcout << std::endl;

	test(L"lorem ipsom dalar");
	test(L"lorem ipsom dalar      ");
	test(L"   lorem ipsom dalar   ");
	test(L"      lorem ipsom dalar");
	*/

	/*
	URL url(L"http:// www.google.de :8080/file.html?lol=rofl");

	std::wcerr << url.getScheme()    << std::endl;
	std::wcerr << url.getHostName()  << std::endl;
	std::wcerr << url.getPort()      << std::endl;
	std::wcerr << url.getUserName()  << std::endl;
	std::wcerr << url.getPassword()  << std::endl;
	std::wcerr << url.getUrlPath()   << std::endl;
	std::wcerr << url.getExtraInfo() << std::endl;
	std::wcerr << std::endl;
	std::wcerr << url.toString()     << std::endl;
	std::wcerr << std::endl;

	URL url2;
	url2.setHostName(L" google.de ");
	url2.setUrlPath(L"asdasd");
	std::wcerr << url2.toString()    << std::endl;
	std::wcerr << std::endl;
	*/

	return 0; //getchar();
}

