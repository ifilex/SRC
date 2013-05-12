@ECHO OFF
echo aPAK v 1.0 - GNU GLP SOURCE
echo Empaquetador - generador de instalaciones XME para programas y productos
echo 2/OS CE bajo plataforma de consola MS DOS y compatibles.
echo.
echo APAK [ARCHIVO.XME] [DIRECTORIO_A_COMPRIMIR] [ARCHIVO_INDEX]
echo.
echo [ARCHIVO.XME]            - Archivo a generar XME
echo [DIRECTORIO_A_COMPRIMIR] - Directorio que se comprimira
echo [ARCHIVO_INDEX]          - Archivo BAT que contendra la informacion sobre
echo                            la instalacion del producto.
echo                            Seria el lanzador de instalacion, el mismo
echo                            puede estar vacio o lleno, dependiendo si se
echo                            quiere dar un mensaje o no, o setear algo.
REM if %1==%2 goto error
if not exist %1 goto do
goto end

:do
cls
echo Generando paquete %1
arj a mrtxs.arj %2 -r               >NUL
echo Generando archivos temporales..
MD TMP
if not exist mrtxs.arj goto error
arj x mrtxs.arj tmp -d -y           >NUL
echo Agregando instalador..
del mrtxs.arj                       >NUL
copy %3 tmp\index.bat               >NUL
cd tmp
if not exist index.bat goto error
arj a ../xxx.arj -r                 >NUL
cd..
deltree /Y tmp                      >NUL
if not exist xxx.arj goto error
arj f -je xxx.arj                   >NUL
del xxx.arj                         >NUL
ren xxx.exe %1                      >NUL
echo Fin del proceso.
goto end

:error
echo Ha ocurrido un error interno, cuando se realizaba el paquete..
goto end

:end
