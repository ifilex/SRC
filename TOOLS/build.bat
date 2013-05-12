@echo off
Make -fEXE2BIN
echo Compilando write.c
bcc -I..\include -L..\libs write.c
upx -9 write.exe
