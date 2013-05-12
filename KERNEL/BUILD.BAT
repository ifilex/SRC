@echo off
echo                    Copyright (c) 1995, 1998
echo                       Pasquale J. Villani
echo                       All Rights Reserved
echo.
echo This file is part of 2OS-CE.
echo.
echo 2OS-CE is free software; you can redistribute it and/or
echo modify it under the terms of the GNU General Public License
echo as published by the Free Software Foundation; either version
echo 2, or (at your option) any later version.
echo.
echo 2OS-CE is distributed in the hope that it will be useful, but
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

CD DRIVERS
CALL BUILD.BAT
CD ..
