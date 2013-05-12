@echo off
:ini
echo.
echo Writer Imager 1.2 for 2OS
echo GNU - GLP 2.0 - www.gnu.org
echo.
echo Comando:
echo.
echo ewrite [imagen] [unidad:]
echo.
echo  [imagen] Archivo PAK de imagen instalatoria.
echo  [unidad] Unidad donde se instalara. (ej a:)
echo.

if exist %1 goto img
goto end

:img
cls
copy %1 x.exe >NUL
if not exist x.exe goto error  >NUL
echo.
echo Espere porfavor, descompactando imagen..
echo.
x -e -y %2\        >NUL
x -e -y sbin\*.*   >NUL
cd sbin
if not exist trans.com goto error >NUL
trans %2
cd..
deltree /Y sbin >NUL
del x.exe       >NUL
goto end

:error
echo.
echo ATENCION!! Archivo corrupto!!
echo.
echo Es probable que el formato de la imagen no sea soportada o bien que no
echo sea un archivo de imagen.
echo Los archivos de imagenes 2/OS CE son de extencion PAK y formato ARC.
echo.
goto end


:end
