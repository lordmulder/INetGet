@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM // INetGet - Lightweight command-line front-end to WinINet API
REM // Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
REM ///////////////////////////////////////////////////////////////////////////

call "%~dp0\z_paths.bat"
cd /d "%~dp0"

echo ---------------------------------------------------------------------
echo GENERATING DOCS
echo ---------------------------------------------------------------------
echo.

for %%i in (*.md) do (
	echo %%~ni --^> %%~ni.local.html
	"%INETGET_PDOC_PATH%\pandoc.exe" --from markdown_github+pandoc_title_block+header_attributes+implicit_figures --to html5 -H "%~dp0\etc\doc\Style.inc" --standalone "%%~i" --output "%~dp0\%%~ni.local.html"
	echo.
)
