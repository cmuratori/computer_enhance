#!/bin/bash

# Create 'build' directory if it does not exist
if [ ! -d build ]; then
  mkdir build
fi
pushd build

# Check if `nasm` is in the PATH and compile with `nasm` if available
if command -v nasm &> /dev/null; then
  # Extract the filename without extension from the first argument
  filename=$(basename -- "$1")
  extension="${filename##*.}"
  filename="${filename%.*}"

  # Compile with nasm
  nasm -f elf64 -o "${filename}.o" ../"$1"

  # Create a static library from the object file, using 'ar' as a likely substitute for 'lib'
  ar rcs "${filename}.a" "${filename}.o"
fi

popd

