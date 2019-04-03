set PATH=%PATH%;%CD%\..\.ci\bin
make
exit /b %errorlevel%
