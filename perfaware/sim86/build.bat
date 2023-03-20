@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -FC ..\sim86.cpp -Fesim86_msvc_debug.exe
call clang -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_debug.exe
call cl -O2 -nologo -Zi -FC ..\sim86.cpp -Fesim86_msvc_release.exe
call clang -O3 -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_release.exe

call clang -P -E ..\sim86_lib.h | call clang-format --style="Microsoft" > ..\shared\sim86_shared.h
call clang -P -E ..\sim86_instruction_table_standalone.h | call clang-format --style="Microsoft" > sim86_instruction_table_standalone.h

call cl -nologo -Zi -FC ..\sim86_lib.cpp -Fesim86_shared_debug.dll /link /DLL /PDBALTPATH:sim86_shared_debug.pdb /export:Sim86_Decode8086Instruction /export:Sim86_RegisterNameFromOperand /export:Sim86_MnemonicFromOperationType /export:Sim86_Get8086InstructionTable /export:Sim86_GetVersion
call cl -nologo -O2 -Zi -FC ..\sim86_lib.cpp -Fesim86_shared_release.dll /link /DLL /PDBALTPATH:sim86_shared_release.pdb /export:Sim86_Decode8086Instruction /export:Sim86_RegisterNameFromOperand /export:Sim86_MnemonicFromOperationType /export:Sim86_Get8086InstructionTable /export:Sim86_GetVersion

call copy sim86_shared*.dll ..\shared
call copy sim86_shared*.lib ..\shared
call copy sim86_shared*.pdb ..\shared

call cl -nologo -Zi -FC ..\shared\shared_library_test.cpp -Feshared_library_test.exe /link

popd
