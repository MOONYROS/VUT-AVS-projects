#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-24-108
#SBATCH -n 1 
#SBATCH --comment "use:vtune=2022.2.0"
#SBATCH -t 0:25:00
#SBATCH --mail-type END
#SBATCH -J AVS-advisor


cd $SLURM_SUBMIT_DIR
ml purge
ml VTune Advisor intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0

[ -d build_advisor ] && rm -rf build_advisor
[ -d build_advisor ] || mkdir build_advisor
cd build_advisor

CC=icx CXX=icpx cmake ..
make


for calc in "ref" "batch" "line"; do
    rm -rf Advisor-$calc
    mkdir Advisor-$calc

    # Basic survey
    advixe-cl -collect survey -project-dir Advisor-$calc  -- ./mandelbrot -c $calc -s 4096


    # Roof line
    advixe-cl -collect tripcounts -flop -project-dir Advisor-$calc  -- ./mandelbrot -c $calc -s 4096
done
