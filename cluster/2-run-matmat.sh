#!/bin/bash
#SBATCH -J matmat_run
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1

mkdir -p logs

NAME="matmat"
PROGRAM="/home/user04/pangeran_bercuda_mpi/2-$NAME"

for N in 512 1024 2048 4096; do
    for NP in 2 4 8 16 32; do
        NODES=$(( (NP + 7) / 8 ))
        echo "Running N=$N with NP=$NP on $NODES node(s)..."

        srun --mpi=pmix -N $NODES -n $NP --ntasks-per-node=8 --cpu-bind=cores \
            $PROGRAM $N 2>&1 | tee logs/2-${NAME}-results-N${N}_NP${NP}.txt

        echo "===================="
    done
done
