#!/bin/bash
#SBATCH -N 4
#SBATCH --ntasks-per-node=8

mkdir -p logs

for NP in 2 4 8 16 32; do
  echo "Running with NP=$NP"
  srun -n $NP /home/user04/pangeran_bercuda_mpi/1-hello_mpi 2>&1 \
    | tee logs/1-hello_mpi-results-NP${NP}.txt
  echo "===================="
done
