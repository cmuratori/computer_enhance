@echo off
IF NOT EXIST build mkdir build
pushd build
call cl -nologo -Zi -FC ..\sim86.cpp -Fesim86_msvc_debug.exe
call clang -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_debug.exe
call cl -O2 -nologo -Zi -FC ..\sim86.cpp -Fesim86_msvc_release.exe
call clang -O3 -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_release.exe
popd
