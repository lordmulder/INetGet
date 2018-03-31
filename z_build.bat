@echo off
setlocal enabledelayedexpansion

REM ///////////////////////////////////////////////////////////////////////////
REM // INetGet - Lightweight command-line front-end to WinINet API
REM // Copyright (C) 2018 LoRd_MuldeR <MuldeR2@GMX.de>
REM ///////////////////////////////////////////////////////////////////////////

echo Preparing to build INetGet, please wait...
echo.

REM ///////////////////////////////////////////////////////////////////////////
REM // Check paths
REM ///////////////////////////////////////////////////////////////////////////

cd /d "%~dp0"
call "%~dp0\z_paths.bat"

if not exist "%INETGET_MSVC_PATH%\vcvarsall.bat" (
	echo Visual C++ compiler not found. Please check your INETGET_MSVC_PATH var^^!
	goto BuildError
)

if not exist "%INETGET_UPX3_PATH%\upx.exe" (
	echo UPX binary could not be found. Please check your INETGET_UPX3_PATH var^^!
	goto BuildError
)

if not exist "%INETGET_PDOC_PATH%\pandoc.exe" (
	echo Pandoc app could not be found. Please check your INETGET_PDOC_PATH var^^!
	goto BuildError
)

if not exist "%INETGET_HTMC_PATH%\htmlcompressor-1.5.3.jar" (
	echo Html Compressor was not found. Please check your INETGET_HTMC_PATH var^^!
	goto BuildError
)

if not exist "%JAVA_HOME%\bin\java.exe" (
	echo Java runtime executable was not found. Please check your JAVA_HOME var^^!
	goto BuildError
)

REM ///////////////////////////////////////////////////////////////////////////
REM // Setup environment
REM ///////////////////////////////////////////////////////////////////////////

set "PATH=%INETGET_HTMC_PATH%;%PATH%"
call "%INETGET_MSVC_PATH%\vcvarsall.bat" x86

REM ///////////////////////////////////////////////////////////////////////////
REM // Get current date and time (in ISO format)
REM ///////////////////////////////////////////////////////////////////////////

set "ISO_DATE="
set "ISO_TIME="

for /F "tokens=1,2 delims=:" %%a in ('"%~dp0\etc\bin\date.exe" +ISODATE:%%Y-%%m-%%d') do (
	if "%%a"=="ISODATE" set "ISO_DATE=%%b"
)

for /F "tokens=1,2,3,4 delims=:" %%a in ('"%~dp0\etc\bin\date.exe" +ISOTIME:%%T') do (
	if "%%a"=="ISOTIME" set "ISO_TIME=%%b:%%c:%%d"
)

if "%ISO_DATE%"=="" goto BuildError
if "%ISO_TIME%"=="" goto BuildError

REM ///////////////////////////////////////////////////////////////////////////
REM // Detect version number
REM ///////////////////////////////////////////////////////////////////////////

set "VER_MAJOR="
set "VER_MINOR="
set "VER_PATCH="

for /F "tokens=1,2,3" %%a in (%~dp0\src\Version.h) do (
	if "%%a"=="#define" (
		if "%%b"=="VER_INETGET_MAJOR" set "VER_MAJOR=%%c"
		if "%%b"=="VER_INETGET_MIN_H" set "VER_MINOR=%%c!VER_MINOR!"
		if "%%b"=="VER_INETGET_MIN_L" set "VER_MINOR=!VER_MINOR!%%c"
		if "%%b"=="VER_INETGET_PATCH" set "VER_PATCH=%%c"
	)
)

if "%VER_MAJOR%"=="" goto BuildError
if "%VER_MINOR%"=="" goto BuildError
if "%VER_PATCH%"=="" goto BuildError

echo Version: %VER_MAJOR%.%VER_MINOR%-%VER_PATCH%
echo ISODate: %ISO_DATE%, %ISO_TIME%
echo.

REM ///////////////////////////////////////////////////////////////////////////
REM // Clean Binaries
REM ///////////////////////////////////////////////////////////////////////////

for /d %%d in (%~dp0\bin\*) do (rmdir /S /Q "%%~d")
for /d %%d in (%~dp0\obj\*) do (rmdir /S /Q "%%~d")

REM ///////////////////////////////////////////////////////////////////////////
REM // Build the binaries
REM ///////////////////////////////////////////////////////////////////////////

for %%p in (Win32,x64) do (
	echo ---------------------------------------------------------------------
	echo BEGIN BUILD [%%p/Release]
	echo ---------------------------------------------------------------------
	echo.
	MSBuild.exe /property:Platform=%%p /property:Configuration=Release /target:clean   "%~dp0\INetGet_VC%INETGET_TOOL_VERS%.sln"
	if not "!ERRORLEVEL!"=="0" goto BuildError
	MSBuild.exe /property:Platform=%%p /property:Configuration=Release /target:rebuild "%~dp0\INetGet_VC%INETGET_TOOL_VERS%.sln"
	if not "!ERRORLEVEL!"=="0" goto BuildError
	MSBuild.exe /property:Platform=%%p /property:Configuration=Release /target:build   "%~dp0\INetGet_VC%INETGET_TOOL_VERS%.sln"
	if not "!ERRORLEVEL!"=="0" goto BuildError
	echo.
)

REM ///////////////////////////////////////////////////////////////////////////
REM // Copy program files
REM ///////////////////////////////////////////////////////////////////////////

echo ---------------------------------------------------------------------
echo BEGIN PACKAGING
echo ---------------------------------------------------------------------
echo.

set "PACK_PATH=%TMP%\~%RANDOM%%RANDOM%.tmp"

mkdir "%PACK_PATH%"
mkdir "%PACK_PATH%\img"
mkdir "%PACK_PATH%\img\inetget"
mkdir "%PACK_PATH%\examples"

copy "%~dp0\bin\v%INETGET_TOOL_VERS%\Win32\Release\INetGet.exe" "%PACK_PATH%\INetGet.exe"
if not "!ERRORLEVEL!"=="0" goto BuildError

copy "%~dp0\bin\v%INETGET_TOOL_VERS%\.\x64\Release\INetGet.exe" "%PACK_PATH%\INetGet.x64.exe"
if not "!ERRORLEVEL!"=="0" goto BuildError

copy "%~dp0\img\inetget\*.png" "%PACK_PATH%\img\inetget"
if not "!ERRORLEVEL!"=="0" goto BuildError

copy "%~dp0\etc\examples\*example*" "%PACK_PATH%\examples"
if not "!ERRORLEVEL!"=="0" goto BuildError

echo.

REM ///////////////////////////////////////////////////////////////////////////
REM // Final Processing
REM ///////////////////////////////////////////////////////////////////////////

for %%i in (*.md) do (
	echo %%~ni --^> "%PACK_PATH%\%%~ni.html"
	"%INETGET_PDOC_PATH%\pandoc.exe" --from markdown_github+pandoc_title_block+header_attributes+implicit_figures --to html5 -H "%~dp0\etc\doc\Style.inc" --standalone "%%~i" | "%JAVA_HOME%\bin\java.exe" -jar "%INETGET_HTMC_PATH%\htmlcompressor-1.5.3.jar" --compress-css  --compress-js -o "%PACK_PATH%\%%~ni.html"
	if not "!ERRORLEVEL!"=="0" goto BuildError
)

pause

"%INETGET_UPX3_PATH%\upx.exe" --brute "%PACK_PATH%\*.exe"
if not "!ERRORLEVEL!"=="0" goto BuildError

echo.

REM ///////////////////////////////////////////////////////////////////////////
REM // Create version tag
REM ///////////////////////////////////////////////////////////////////////////

echo INetGet - Lightweight command-line front-end to WinINet API>                      "%PACK_PATH%\BUILD_TAG"
echo Copyright (C) 2018 LoRd_MuldeR ^<MuldeR2@GMX.de^>>>                               "%PACK_PATH%\BUILD_TAG"
echo.>>                                                                                "%PACK_PATH%\BUILD_TAG"
echo Version %VER_MAJOR%.%VER_MINOR%-%VER_PATCH%. Built on %ISO_DATE%, at %ISO_TIME%>> "%PACK_PATH%\BUILD_TAG"
echo.>>                                                                                "%PACK_PATH%\BUILD_TAG"
echo This program is free software; you can redistribute it and/or modify>>            "%PACK_PATH%\BUILD_TAG"
echo it under the terms of the GNU General Public License as published by>>            "%PACK_PATH%\BUILD_TAG"
echo the Free Software Foundation; either version 2 of the License, or>>               "%PACK_PATH%\BUILD_TAG"
echo (at your option) any later version.>>                                             "%PACK_PATH%\BUILD_TAG"
echo.>>                                                                                "%PACK_PATH%\BUILD_TAG"
echo This program is distributed in the hope that it will be useful,>>                 "%PACK_PATH%\BUILD_TAG"
echo but WITHOUT ANY WARRANTY; without even the implied warranty of>>                  "%PACK_PATH%\BUILD_TAG"
echo MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the>>                   "%PACK_PATH%\BUILD_TAG"
echo GNU General Public License for more details.>>                                    "%PACK_PATH%\BUILD_TAG"

REM ///////////////////////////////////////////////////////////////////////////
REM // Attributes
REM ///////////////////////////////////////////////////////////////////////////

attrib +R "%PACK_PATH%\*"
attrib +R "%PACK_PATH%\img\inetget\*"

REM ///////////////////////////////////////////////////////////////////////////
REM // Generate outfile name
REM ///////////////////////////////////////////////////////////////////////////

mkdir "%~dp0\out" 2> NUL
set "OUT_NAME=WinINetGet.%ISO_DATE%"

:CheckOutName
if exist "%~dp0\out\%OUT_NAME%.zip" (
	set "OUT_NAME=%OUT_NAME%.new"
	goto CheckOutName
)

REM ///////////////////////////////////////////////////////////////////////////
REM // Build the package
REM ///////////////////////////////////////////////////////////////////////////

pushd "%PACK_PATH%\"
"%~dp0\etc\bin\zip.exe" -9 -r -z "%~dp0\out\%OUT_NAME%.zip" "*.*" < "%PACK_PATH%\BUILD_TAG"
popd
attrib +R "%~dp0\out\%OUT_NAME%.zip"

REM Clean up!
rmdir /Q /S "%PACK_PATH%"

REM ///////////////////////////////////////////////////////////////////////////
REM // COMPLETE
REM ///////////////////////////////////////////////////////////////////////////

echo.
echo Build completed.
echo.

pause
goto:eof

REM ///////////////////////////////////////////////////////////////////////////
REM // FAILED
REM ///////////////////////////////////////////////////////////////////////////

:BuildError

echo.
echo Build has failed ^^!^^!^^!
echo.

pause
goto:eof
