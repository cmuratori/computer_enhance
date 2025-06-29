@echo off

IF NOT EXIST build mkdir build
pushd build

where /q nasm && (
  call nasm -f win64 -o %~n1.obj ..\%1
  call lib -nologo %~n1.obj
)

popd
