#!/bin/bash

if [ -z "${CXX}" ]; then
    echo "WARNING: Environment variable CXX not set."
    exit
fi
# Check if `${CXX}` is in the PATH
if ! command -v ${CXX} &> /dev/null; then
    echo "WARNING: ${CXX} not found -- executables will not be built"
    echo "Set CXX environment variable to a working C++ compiler."
    exit
fi

# Set buildpat based on the first script argument
if [ -z "$1" ]; then
    for g in ./*_main.cpp ; do
        ./build_single.sh "$g"
    done
else
    for g in ./*$1*_main.cpp ; do
        ./build_single.sh "$g"
    done
fi
