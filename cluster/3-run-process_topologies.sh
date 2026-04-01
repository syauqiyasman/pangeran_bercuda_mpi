#!/bin/bash
#SBATCH -J process_topologies_run
#SBATCH --cpus-per-task=1

mkdir -p logs

PROGRAM="/home/user04/pangeran_bercuda_mpi/3-process_topologies"
MAX_CORES=8

for NP in 2 4 8 16 32; do
    echo "Running with NP=$NP..."

    srun --mpi=pmix -n $NP --cpu-bind=cores \
        $PROGRAM 2>&1 | tee logs/3-process_topologies-results-NP${NP}.txt

    echo "===================="
done