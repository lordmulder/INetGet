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

//Win32
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <MMSystem.h>

//CRT
#include <iostream>
#include <process.h>
#include <io.h>
#include <fcntl.h>

//Internal
#include "Sync.h"

//Validation
#if (defined(NDEBUG) && defined(_DEBUG)) || ((!defined(NDEBUG)) && (!defined(_DEBUG)))
#error Inconsistent DEBUG flags!
#endif

//Events
static Sync::Event g_eventUserAbort;
namespace Zero
{
	Sync::Signal g_sigUserAbort(g_eventUserAbort);
}

//=============================================================================
// ERROR HANDLING
//=============================================================================

static BOOL WINAPI my_sigint_handler(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		g_eventUserAbort.set(true);
		return TRUE;
	}
	return FALSE;
}

#ifdef NDEBUG

static void my_invalid_param_handler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
{
	std::wcerr << L"\n\nGURU MEDITATION: Invalid parameter handler invoked, application will exit!\n" << std::endl;
	_exit(EXIT_FAILURE);
}

static LONG WINAPI my_exception_handler(struct _EXCEPTION_POINTERS* /*ExceptionInfo*/)
{
	std::wcerr << L"\n\nGURU MEDITATION: Unhandeled exception handler invoked, application will exit!\n" << std::endl;
	_exit(EXIT_FAILURE);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void setup_error_handlers(void)
{
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	SetUnhandledExceptionFilter(my_exception_handler);
	_set_invalid_parameter_handler(my_invalid_param_handler);

	if(const HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll"))
	{
		typedef BOOL (WINAPI *SetDefaultDllDirectoriesT)(DWORD flags);
		if(const SetDefaultDllDirectoriesT set_default_dll_directories = (SetDefaultDllDirectoriesT) GetProcAddress(hKernel32, "SetDefaultDllDirectories"))
		{
			set_default_dll_directories(0x0800); /*LOAD_LIBRARY_SEARCH_SYSTEM32*/
		}
		typedef BOOL (WINAPI *SetDllDirectoryT)(LPCWSTR lpPathName);
		if(const SetDllDirectoryT set_dll_directory = (SetDllDirectoryT) GetProcAddress(hKernel32, "SetDllDirectoryW"))
		{
			set_dll_directory(L""); /*remove current directory from search path*/
		}
	}
}

#endif //NDEBUG

//=============================================================================
// STARTUP
//=============================================================================

int inetget_main(const int argc, const wchar_t *const argv[]);

static int inetget_startup(const int argc, const wchar_t *const argv[])
{
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stderr), _O_U8TEXT);
	_setmode(_fileno(stdin ), _O_BINARY);

	timeBeginPeriod(1);
	SetConsoleCtrlHandler(my_sigint_handler, TRUE);

	return inetget_main(argc, argv);
}

//=============================================================================
// ENTRY POINT
//=============================================================================

#ifdef NDEBUG
static int wmain_ex(const int argc, const wchar_t *const argv[])
{
	int ret = -1;
	try
	{
		ret = inetget_startup(argc, argv);
	}
	catch(std::exception &err)
	{
		std::wcerr << L"\n\nUNHANDELED EXCEPTION: " << err.what() << '\n' << std::endl;
		_exit(EXIT_FAILURE);
	}
	catch(...)
	{
		std::wcerr << L"\n\nUNHANDELED EXCEPTION: Unknown C++ exception error!\n" << std::endl;
		_exit(EXIT_FAILURE);
	}
	return ret;
}
#endif //NDEBUG

int wmain(int argc, wchar_t* argv[])
{
	int ret = -1;
#ifdef NDEBUG
	__try
	{
		setup_error_handlers();
		ret = wmain_ex(argc, argv);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		std::wcerr << L"\n\nGURU MEDITATION: Unhandeled exception error, application will exit!\n" << std::endl;
	}
#else
	ret =  inetget_startup(argc, argv);
#endif
	return ret;
}
