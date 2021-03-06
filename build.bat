@echo off

set INPUT_DIR=%~dp0
echo %INPUT_DIR%
pushd %INPUT_DIR% || goto :error
setlocal enabledelayedexpansion
if "%1" == "clean" (
rmdir /s /q bin
rmdir /s /q atlas_include
exit /b 0
)

set /A _tempvar=0
If "%1" == "full" set /A _tempvar=1
If "%1" == "release" set /A _tempvar=1
if %_tempvar% EQU 1 (
rmdir /s /q bin 2> NUL
mkdir bin || goto :mkdirerror
pushd bin || goto :error
robocopy /e /nfl /ndl %INPUT_DIR%\libs\SDL2-2.0.10\lib\x64\ . 
popd || goto :error
rmdir /s /q atlas_include 2> NUL
mkdir atlas_include || goto :mkdirerror
)

pushd art || goto :error
..\tools\packer\bin\packer.exe pcx settings_atlas.pcx SettingsAtlas ..\atlas_include\settings_atlas.h || goto :error
popd || goto :error

pushd bin || goto :error

set OUTPUT_FILE=gep.exe
set WARNINGS=-WX -W4 -wd4100 -wd4189 -wd4201 -wd4996 -wd4204 -wd4221 -wd4200
set INCLUDE_DIR=-I%INPUT_DIR%\libs\SDL2-2.0.10\include
set MACRO_SWITCHES=
set COMPILER_OPTIONS=-Oi -fp:fast
set DEBUG_OPTIONS=-Zi
set LINKER_DEBUG_OPTIONS =-profile
set OPTIMIZATION_LEVEL=-Od
rem d - none
rem 1 - small code
rem 2 - fast code
if "%1" == "release" (
set OPTIMIZATION_LEVEL=-O2
set DEBUG_OPTIONS=
set LINKER_DEBUG_OPTIONS =
)
cl %INCLUDE_DIR% %MACRO_SWITCHES% %WARNINGS% %OPTIMIZATION_LEVEL% %COMPILER_OPTIONS% %DEBUG_OPTIONS% "%INPUT_DIR%\sdl_win_main.c" SDL2.lib SDL2main.lib User32.lib -link -incremental:no -subsystem:windows -opt:ref -out:%OUTPUT_FILE% %LINKER_DEBUG_OPTIONS% || goto :error

popd || goto :error
popd || goto :error

goto :EOF
:error
echo ABORT!
exit /b %errorlevel%

:mkdirerror
echo Failed to make /bin directory, this can happen if the bin dir was open in a file explorer window.
echo ABORT!
exit /b %errorlevel%