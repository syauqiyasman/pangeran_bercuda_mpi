#!/bin/bash
#SBATCH -N 1
#SBATCH -n 32
#SBATCH --cpus-per-task=1

mkdir -p logs

for NP in 2 4 8 12 16 24 32; do
  echo "Running topology_demo with NP=$NP"
  mpirun -np $NP --bind-to core /home/user04/pangeran_bercuda_mpi/3-process_topologies 2>&1 \
    | tee logs/3-process_topologies-results-NP${NP}.txt
  echo "===================="
done
