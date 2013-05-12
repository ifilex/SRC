@echo off
ECHO.
ECHO Compilando KERNEL.SYS
ECHO.
TMAKE -f2os.mak
DEL *.map
DEL *.exe
rem DEL *.sys
DEL *.obj
MOVE KERNEL.SYS ..\..\..\..\BOOT

