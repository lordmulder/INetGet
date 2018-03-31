@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM // INetGet - Lightweight command-line front-end to WinINet API
REM // Copyright (C) 2018 LoRd_MuldeR <MuldeR2@GMX.de>
REM ///////////////////////////////////////////////////////////////////////////

call "%~dp0\z_paths.bat"
cd /d "%~dp0"
set "PATH=%INETGET_HTMC_PATH%;%PATH%"

echo ---------------------------------------------------------------------
echo GENERATING DOCS
echo ---------------------------------------------------------------------
echo.

for %%i in (*.md) do (
	echo %%~ni --^> %%~ni.local.html
	"%INETGET_PDOC_PATH%\pandoc.exe" --from markdown_github+pandoc_title_block+header_attributes+implicit_figures --to html5 -H "%~dp0\etc\doc\Style.inc" --standalone "%%~i" | "%JAVA_HOME%\bin\java.exe" -jar "%INETGET_HTMC_PATH%\htmlcompressor-1.5.3.jar" --compress-css  --compress-js -o "%~dp0\%%~ni.local.html"
	echo.
)
