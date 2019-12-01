@echo off

set INPUT_DIR=%~dp0
echo %INPUT_DIR%
pushd %INPUT_DIR% || goto :error
setlocal enabledelayedexpansion
if "%1" == "clean" (
rmdir /s /q bin
exit /b 0
)
set /A _tempvar=0
if "%1" == "full" set /A _tempvar=1
if "%1" == "release" set /A _tempvar=1
if %_tempvar% EQU 1 (
rmdir /s /q bin 2> NUL
mkdir bin || goto :mkdirerror
pushd bin || goto :error
popd || goto :error
)

pushd bin || goto :error

set OUTPUT_FILE=packer.exe
set WARNINGS=-WX -W4 -wd4100 -wd4189 -wd4201 -wd4996 -wd4204 -wd4221 -wd4200
set INCLUDE_DIR=
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
cl %INCLUDE_DIR% %MACRO_SWITCHES% %WARNINGS% %OPTIMIZATION_LEVEL% %COMPILER_OPTIONS% %DEBUG_OPTIONS% "%INPUT_DIR%\packer.c" -link -incremental:no -subsystem:console -opt:ref -out:%OUTPUT_FILE% %LINKER_DEBUG_OPTIONS% || goto :error
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