@echo off
set PATH=%CD%\..\tools\build_tools;%PATH%
make
exit /b %errorlevel%
