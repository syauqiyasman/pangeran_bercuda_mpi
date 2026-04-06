#!/bin/bash
#SBATCH -o run-4.out
#SBATCH -p batch
#SBATCH -N 4
#SBATCH --nodelist=node-01,node-03,node-04,node-06

mkdir -p logs

PROGRAM="/home/user04/pangeran_bercuda_mpi/1-hello_mpi"

for NP in 2 4 8 16 32; do
    echo "Running with NP=$NP..."

    mpirun --oversubscribe --mca btl_top_if_exclude docker0,lo -np $NP \
        $PROGRAM 2>&1 | tee logs/1-hello_mpi-results-NP${NP}.txt

    echo "===================="
done
