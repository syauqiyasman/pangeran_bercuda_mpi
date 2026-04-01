#!/bin/bash
#SBATCH -N 4
#SBATCH --ntasks-per-node=8

mkdir -p logs

for N in 512 1024 2048 4096; do
  for NP in 2 4 8 16 32; do
    echo "Running N=$N with NP=$NP"
    srun -n $NP /home/user04/pangeran_bercuda_mpi/2-matmat $N 2>&1 \
      | tee logs/2-matmat-results-N${N}_NP${NP}.txt
    echo "===================="
  done
done
