@echo off

set arg=%1

set ALL_INPUT_DIR=%~dp0
echo %ALL_INPUT_DIR%
cd /D %ALL_INPUT_DIR% || goto :error
echo building packer.exe...
call "tools\packer\build.bat" %arg% || goto :error
echo finished building packer.exe
cd /D %ALL_INPUT_DIR% || goto :error
setlocal enabledelayedexpansion
echo building gep.exe...
call "build.bat" %arg% || goto :error
echo finished building gep.exe
cd /D %ALL_INPUT_DIR% || goto :error
echo building demos...
echo    building flappy...
call "demos\flappy\build.bat" %arg% || goto :error
echo    finished building flappy
echo finished building demos

goto :EOF
:error
echo ABORT!
exit /b %errorlevel%