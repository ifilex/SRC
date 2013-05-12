@ECHO OFF
echo xPAK 1.1 - GNU GLP SOURCE
echo Programa de instalacion de paquetes XME (PAK 2/OS CE), que fueron creados
echo con aPAK para instalacion de E/OS.
echo.
echo XPAK [ARCHIVO.XME] [UNIDAD:]
echo.
echo      [ARCHIVO.XME] Es el paquete ARC que sera instalado.
echo      [UNIDAD:]     Unidad donde se lo instalara  (ej c:)
echo.
if exist %1 goto unpak
goto end
:unpak
CLS
echo Desempaquetando..
copy %1 1101.exe         >NUL
if not exist 1101.exe goto error
1101.exe -e -y %2\       >NUL
1101.exe -e -y index.bat >NUL
if not exist index.bat goto error
echo Iniciando instalacion/seteo del paquete..
call index.bat %1 %2          
del index.bat            >NUL
del %2\index.bat         >NUL
del 1101.exe             >NUL
echo Se instalo el paquete %1 en la unidad %2
echo.
echo Fin de la instalacion.
goto end

:error
echo.
echo ERROR: El paquete se encuentra corrupto, o bien no es un formato ARC
echo correspondiente, verifiquelo o vuelva a bajarlo.
echo.
goto end

:end
