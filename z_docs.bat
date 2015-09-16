@echo off
call "%~dp0\z_paths.bat"

cd /d "%~dp0"
for %%i in (*.md) do (
	echo [%%~ni]
	"%INETGET_PANDOC_PATH%\pandoc.exe" --from markdown_github+pandoc_title_block+header_attributes+implicit_figures --to html5 --standalone "%%~i" --output "%~dp0\%%~ni.local.html"
)
