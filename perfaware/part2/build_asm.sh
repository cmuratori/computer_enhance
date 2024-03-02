#!/bin/bash

# Check if `nasm` is in the PATH
if ! command -v nasm &> /dev/null; then
    echo "WARNING: nasm not found -- ASM libs will not be built"
fi

# Set buildpat based on the first script argument
if [ -z "$1" ]; then
    buildpat="*.asm"
    for g in ./*asm ; do
        ./build_single_asm.sh "$g"
    done
else
    for g in ./*$1*.asm ; do
        ./build_single_asm.sh "$g"
    done
fi

