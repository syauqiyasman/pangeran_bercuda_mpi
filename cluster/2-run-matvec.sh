#!/bin/bash
#SBATCH -o run-4.out
#SBATCH -p batch
#SBATCH -N 4
#SBATCH --nodelist=node-01,node-03,node-04,node-06

mkdir -p logs

NAME="matvec"
PROGRAM="/home/user04/pangeran_bercuda_mpi/2-$NAME"

for N in 512 1024 2048 4096; do
    for NP in 2 4 8 16 32; do
        echo "Running N=$N with NP=$NP"

        mpirun --mca btl_tcp_if_exclude docker0,lo -np $NP \
            $PROGRAM $N 2>&1 | tee logs/2-${NAME}-results-N${N}_NP${NP}.txt

        echo "===================="
    done
done
