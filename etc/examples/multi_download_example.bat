@echo off
set "PYTHON_DIR=C:\Program Files\Python36"
set "PATH=%PYTHON_DIR%,%PATH%"
cd /d "%~dp0"
"%PYTHON_DIR%\python.exe" "%~dpn0.py" %1 %2 %3
set RESULT=%ERRORLEVEL%
exit /b %RESULT%
