@echo off

setlocal 

set buildtype=%2
if "%~2"=="" set buildtype=rc
for %%g in (build\*%1_*_%buildtype%.exe) DO (
	echo %%g
	call %%g %3 %4 %5 %6 %7 %8 %9
)

endlocal
