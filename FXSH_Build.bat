@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.

if exist debug\*.obj  del debug\*.obj
if exist VIMS3D.G1A  del VIMS3D.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"C:\CasioSDK\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"C:\CasioSDK\Tools\MakeAddinHeader363.exe" "C:\CasioSDK\Projects\VIMS3D"
if not exist VIMS3D.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

