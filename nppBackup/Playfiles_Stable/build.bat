@echo off
set PATH=%CD%\..\..\tools\build_tools;%PATH%
make all
exit /b %errorlevel%
