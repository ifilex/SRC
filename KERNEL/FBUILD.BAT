@echo off
echo.
echo Compilando Kernel.sys ..
echo.
echo Version 1.0
echo.
echo.
CD ARCH
CALL BUILD.BAT
CD..
CD SCRIPTS
CALL BUILD.BAT
CD ..
CD KERNEL
CALL BUILD.BAT
CD ..

CD ETC
COPY INITTAB ..\..\..\..\etc >NUL
CD ..
