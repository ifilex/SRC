@ECHO OFF
ECHO.
ECHO 2OS CE
ECHO ------
ECHO.
ECHO Se ha iniciado la compilacion del sistema operativo.
ECHO.
cd kernel
call fbuild.bat
cd..
cd bin
call build.bat
cd..
cd libs
call build.bat
cd..
