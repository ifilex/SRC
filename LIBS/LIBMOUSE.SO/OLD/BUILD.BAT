@ECHO OFF
rem Version en ingles
tasm /m9 /ml mouse.asm
tlink /t /C /n mouse,USA\ctmouse
rem Version en espa¤ol
tasm /m9 /ml /dLANG_PT_ES mouse.asm
tlink /t /C /n mouse,mouse

rem ---------------------
rem Compila driver PS/2

rem Version en ingles
tasm /m9 /ml /dPS2=1 mouse.asm,mousep
tlink /t /C /n mousep,USA\mousep
rem Version en espa¤ol
tasm /m9 /ml /dPS2=1 /dLANG_PT_ES mouse.asm,mousep
tlink /t /C /n mousep,mousep
del *.obj
del *.map
upx -9 mouse.com
upx -9 mousep.com
move mouse.com ..\..\..\lib
move mousep.com ..\..\..\lib
