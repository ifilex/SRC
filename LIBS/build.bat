@echo off
echo.
echo Iniciando compilacion de librerias.
echo.
cd libcdrom.so
call build.bat
cd ..
cd libdpmi.so
call build.bat
cd..
cd libems.so
call build.bat
cd..
cd libmouse.so
call build.bat
cd..
cd libshell.so
call build.bat
cd..
cd libsound.so
rem call build.bat
cd..
cd libxms.so
call build.bat
cd..
