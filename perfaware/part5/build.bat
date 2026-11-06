@echo off

where /q cl || (echo WARNING: cl not found -- MSVC executable will not be built)
where /q clang++ || (echo WARNING: clang++ not found -- CLANG executables will not be built)

setlocal 

set buildpat=*%1*_main.cpp
if "%~1"=="" set buildpat=*_main.cpp
for %%g in (%buildpat%) DO (call build_single.bat %%g)

endlocal
