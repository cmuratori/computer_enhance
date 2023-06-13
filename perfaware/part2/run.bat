@echo off

setlocal 

set buildtype=%2
set buildname=
if "%~2"=="" set buildtype=r?
if not "%~1"=="" for %%g in (build\*%1_*_%buildtype%.exe) DO (
	set buildname=%%g
)

if not "%buildname%"=="" (
	echo.
	echo ============ %buildname% ============ 
	echo.
	call %buildname%
)

if "%buildname%"=="" echo No matching executable.

endlocal
