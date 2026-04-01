#!/bin/bash
#SBATCH -J hello_mpi_run
#SBATCH --cpus-per-task=1

mkdir -p logs

PROGRAM="/home/user04/pangeran_bercuda_mpi/1-hello_mpi"

for NP in 2 4 8 16 32; do
    echo "Running with NP=$NP..."

    srun --mpi=pmix -n $NP --cpu-bind=cores \
        $PROGRAM 2>&1 | tee logs/hello_mpi-NP${NP}.txt

    echo "===================="
done
