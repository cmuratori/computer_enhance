@echo off

where /q nasm || (echo WARNING: nasm not found -- ASM libs wil not be built)

setlocal

set buildpat=*%1*.asm
if "%~1"=="" set buildpat=*.asm
for %%g in (%buildpat%) DO (call build_single_asm.bat %%g)

endlocal
