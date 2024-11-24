#!/bin/bash

ml purge
ml intel-compilers/2024.2.0 

cd build
make -j

ml purge
