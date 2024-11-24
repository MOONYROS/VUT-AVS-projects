#!/bin/bash

ml purge
ml matplotlib CMake/3.27.6-GCCcore-13.2.0 intel-compilers/2024.2.0 

cd build
bash ../scripts/compare.sh

ml purge