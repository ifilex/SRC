@ECHO OFF
REM NOTA:
REM Para compilar este codigo necesitara el programa MASM 5.0, para instalarlo
REM en 2/OS agreguelo en el PATH del AUTOEXEC.BAT ubicado en 2OS\LIB.
REM
REM Muchas gracias por su compresion.
ECHO.
ECHO Compilando libems.so
ECHO.
MASM EMS.ASM, EMS.OBJ, , ;
TLINK EMS.OBJ 
EXE2BIN EMS.EXE ..\..\..\..\LIB\LIBEMS.SO
DEL EMS.EXE
DEL EMS.OBJ
DEL EMS.MAP
DEL EMS.CRF
DEL EMS.LST
