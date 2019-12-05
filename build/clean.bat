@echo off
set PATH=%CD%\..\tools\build_tools;%PATH%
make clean
exit /b %errorlevel%
