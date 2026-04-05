#!/bin/bash
#SBATCH -J jacobi_run
#SBATCH --cpus-per-task=1

mkdir -p logs

NAME="jacobi"
PROGRAM="/home/user04/pangeran_bercuda_mpi/4-$NAME"

for N in 512 1024 2048 4096; do
    for NP in 1 2 4 8 16 32; do
        echo "Running N=$N with NP=$NP"

        srun --mpi=pmix -n $NP --cpu-bind=cores \
            $PROGRAM $N 2>&1 | tee logs/4-${NAME}-results-N${N}_NP${NP}.txt

        echo "===================="
    done
done
