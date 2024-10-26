rm -rf build
ml purge
ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0
mkdir build
cd build
CC=icx CXX=icpx cmake ..
make -j
