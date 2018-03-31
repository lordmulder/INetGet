@echo off
set "PYTHON_DIR=C:\Program Files\Python36"
set "PATH=%PYTHON_DIR%,%PATH%"
cd /d "%~dp0"
"%PYTHON_DIR%\python.exe" "%~dpn0.py" %1 %2 %3 %4 %5 %6 %7 %8 %9
set RESULT=%ERRORLEVEL%
exit /b %RESULT%
