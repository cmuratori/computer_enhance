#!/bin/bash -x

# Create 'build' directory if it does not exist
if [ ! -d build ]; then
  mkdir build
fi
pushd build

# Check if `clang++` is in the PATH and compile with `clang++` if available
if command -v ${CXX} &> /dev/null; then
  # Extract the filename without extension from the first argument
  filename=$(basename -- "$1")
  extension="${filename##*.}"
  filename="${filename%.*}"

  # Compile with clang++
  ${CXX} -mavx2 -g -Wall -fuse-ld=lld ../"$1" -L . -o "${filename}_dc"
  ${CXX} -mavx2 -O3 -g -Wall -fuse-ld=lld ../"$1" -L . -o "${filename}_rc"
fi

popd

