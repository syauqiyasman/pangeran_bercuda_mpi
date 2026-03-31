#!/bin/bash
#SBATCH -N 5
#SBATCH --nodelist=node-01,node-03,node-06,node-07,node-08

mkdir -p logs

for N in 512 1024 2048 4096; do
  for NP in 2 4 8 16 32; do
    echo "Running N=$N with NP=$NP"
    mpirun --mca btl_tcp_if_exclude docker0,lo -np $NP /home/user04/pangeran_bercuda_mpi/2-matvec $N 2>&1 \
      | tee logs/2-matvec-results-N${N}_NP${NP}.txt
    echo "===================="
  done
done
