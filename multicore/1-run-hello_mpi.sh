#!/bin/bash
#SBATCH -N 1
#SBATCH -n 32
#SBATCH --cpus-per-task=1

mkdir -p logs

for NP in 2 4 8 16 32; do
  echo "Running with NP=$NP"
  mpirun -np $NP /home/user04/pangeran_bercuda_mpi/1-hello_mpi 2>&1 \
    | tee logs/1-hello_mpi-results-NP${NP}.txt
  echo "===================="
done
