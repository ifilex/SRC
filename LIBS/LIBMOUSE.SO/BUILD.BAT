@echo off
echo Compilando libmouse.so
tasm/m mouse.asm
tlink/t /x mouse.obj,..\..\..\..\lib\libmouse.exe
com2exe -s512 ..\..\..\..\lib\libmouse.exe ..\..\..\..\lib\libmouse.com
del ..\..\..\..\lib\libmouse.exe
del mouse.obj>nul
