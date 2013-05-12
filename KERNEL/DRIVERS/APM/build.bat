@echo off
echo Compilando APM..

echo Compilando fdapm.asm ..
nasm -o apm.com fdapm.asm
move apm.com ..\..\..\..\..\lib