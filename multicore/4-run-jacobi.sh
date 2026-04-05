#!/bin/bash
#SBATCH -N 1
#SBATCH -n 32
#SBATCH --cpus-per-task=1

mkdir -p logs

for N in 512 1024 2048 4096; do
  for NP in 1 2 4 8 16 32; do
    echo "Running N=$N with NP=$NP"
    mpirun -np $NP /home/user04/pangeran_bercuda_mpi/4-jacobi $N 2>&1 \
      | tee logs/4-jacobi-results-N${N}_NP${NP}.txt
    echo "===================="
  done
done
