@echo off
echo.
echo  Compilando drivers de CDROM.
echo.

nasm -o CDROM.SO -l gcdrom.lst xcdrom.asm
MOVE CDROM.SO ..\..\..\..\LIB
