@ECHO OFF
ECHO.
ECHO Compilando libxms.so
ECHO.
tasm iosmem.asm -zi -m3
tlink iosmem.obj -3 -x -v
exe2bin iosmem.exe ..\..\..\..\LIB\LIBXMS.SO
del iosmem.exe
del iosmem.obj
