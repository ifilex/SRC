@echo off
ECHO.
ECHO Compilando libsound.so
ECHO.
REM COPY CTMMSYS.DRV ..\..\..\..\LIB
copy ctsb16.drv libsound.so
MOVE LIBSOUND.SO  ..\..\..\..\LIB
REM Nota por el momento se emplean los drivers de creative universales.
