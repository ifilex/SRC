@echo off
echo		       Proyect 2OS Compact Edition
echo                    Copyright (c) 2008 - 2012
echo                       All Rights Reserve
echo.
echo This proyect is based on DOS-C, FreeDOS, OpenDOS and Linux
echo sources codes.
echo.
echo 2OS is free software; you can redistribute it and/or
echo modify it under the terms of the GNU General Public License
echo as published by the Free Software Foundation; either version
echo 2, or (at your option) any later version.
echo.
echo 2OS is distributed in the hope that it will be useful, but
echo WITHOUT ANY WARRANTY; without even the implied warranty of
echo MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
echo the GNU General Public License for more details.
echo.
echo You should have received a copy of the GNU General Public
echo License along with DOS-C; see the file COPYING.  If not,
echo write to the Free Software Foundation, 675 Mass Ave,
echo Cambridge, MA 02139, USA.
echo.
echo.
pause
echo Generando booter de arranque.
echo.
echo Generando Raiz..
MD TMP                          >NUL
cd tmp                          >NUL
MD BOOT                         >NUL
MD BIN                          >NUL
MD ETC                          >NUL
CD ETC
MD INIT				>NUL
cd ..
MD LIB                          >NUL
MD SBIN                         >NUL
MD TMP                          >NUL
MD USR                          >NUL
MD VAR                          >NUL
CD USR                          
MD BIN                          >NUL
MD SBIN
MD LIB
MD SHARE
CD SHARE
MD QUADNET
CD..
CD..
CD VAR                          
MD LIB                          >NUL
CD..
echo.
echo Iniciando copia de ficheros..
echo.
copy ..\..\..\boot\*.* boot	    >NUL
copy ..\..\..\bin\*.* bin	    >NUL
copy ..\..\..\etc\*.* etc	    >NUL
copy ..\..\..\etc\init\*.* etc\init >NUL
copy ..\..\..\lib\*.* lib           >NUL
copy ..\..\..\sbin\*.* sbin         >NUL
copy ..\..\..\tmp\*.* tmp           >NUL
copy ..\..\..\usr\bin\*.* usr\bin   >NUL
copy ..\..\..\usr\sbin\*.* usr\sbin >NUL
copy ..\..\..\usr\lib\*.* usr\lib   >NUL
copy ..\..\..\var\lib\*.* var\lib   >NUL
copy ..\..\..\usr\share\quadnet\*.* usr\share\quadnet   >NUL
echo Fin de copia.
echo.
echo Comprimiendo imagen..
arj a boot.arj -r     		 >NUL
arj f -je boot.arj               >NUL
del boot.arj                     >NUL
ren boot.exe boot.pak            >NUL
move boot.pak ..\boots           >NUL
CD..
copy tools\write.exe boots       >NUL
rm -r tmp                        >NUL
rem deltree /Y tmp 
