@echo off

IF NOT EXIST build mkdir build
pushd build

where /q cl && (
  call cl -arch:AVX2 -Zi -W4 -nologo ..\%1 -Fe%~n1_dm.exe
  call cl -arch:AVX2 -O2 -Zi -W4 -nologo ..\%1 -Fe%~n1_rm.exe -Fm%~n1_rm.map
)

where /q clang++ && (
  call clang++ -mavx2 -g -Wall -fuse-ld=lld ..\%1 -o %~n1_dc.exe
  call clang++ -mavx2 -O3 -g -Wall -fuse-ld=lld ..\%1 -o %~n1_rc.exe
)

popd
