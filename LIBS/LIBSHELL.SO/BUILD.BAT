@ECHO OFF
ECHO.
ECHO Compilando libshell.so
ECHO.
tasm cut.asm
tlink cut.obj
del cut.obj
del cut.map
REN CUT.EXE LIBSHELL.EXE
MOVE LIBSHELL.EXE ..\..\..\..\LIB
UPX -9 ..\..\..\..\LIB\LIBSHELL.EXE
