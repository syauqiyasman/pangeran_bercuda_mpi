#!/bin/bash
#SBATCH -J hello_mpi_run
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1

mkdir -p logs

PROGRAM="/home/user04/pangeran_bercuda_mpi/1-hello_mpi"

for NP in 2 4 8 16 32; do
    NODES=$(( (NP + 7) / 8 ))
    echo "Running with NP=$NP on $NODES node(s)..."

    srun --mpi=pmix -N $NODES -n $NP --ntasks-per-node=8 --cpu-bind=cores \
        $PROGRAM 2>&1 | tee logs/hello_mpi-NP${NP}.txt

    echo "===================="
done
