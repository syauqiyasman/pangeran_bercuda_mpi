#!/bin/bash
#SBATCH -N 5
#SBATCH --nodelist=node-01,node-03,node-06,node-07,node-08

mkdir -p logs

for NP in 2 4 8 16 32; do
  echo "Running topology_demo with NP=$NP"
  mpirun --mca btl_tcp_if_exclude docker0,lo -np $NP /home/user04/pangeran_bercuda_mpi/topology_demo 2>&1 \
    | tee logs/3-process_topologies-results-NP${NP}.txt
  echo "===================="
done
