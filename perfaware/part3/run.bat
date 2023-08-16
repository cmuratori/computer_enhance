@echo off

setlocal 

set buildtype=%2
set buildname=
if "%~2"=="" set buildtype=r?
if not "%~1"=="" for %%g in (build\*%1_*_%buildtype%.exe) DO (
	set buildname=%%g
)

if "%buildname%"=="" echo No matching executable.

if not "%buildname%"=="" (
	echo.
	echo ============ %buildname% ============ 
	echo.
	call %buildname% %3 %4 %5 %6 %7 %8 %9
)

endlocal
