#!/bin/bash

ml purge
ml CMake/3.27.6-GCCcore-13.2.0 intel-compilers/2024.2.0 

if [ -d "build" ]; then
    rm -rf build
fi
mkdir build
cd build
CC=icx CXX=icpx cmake ..
make -j

ml purge
