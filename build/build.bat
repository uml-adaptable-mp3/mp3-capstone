@echo off
set PATH=%CD%\..\tools\build_tools;%PATH%
cd ../drivers/Playfiles_Stable
make
exit /b %errorlevel%
