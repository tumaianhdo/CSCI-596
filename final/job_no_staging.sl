#!/bin/bash -l
#SBATCH -t 00:30:00
#SBATCH -N 2
#SBATCH -p debug
#SBATCH -L SCRATCH
#SBATCH -C haswell
## On the Cray, you need at least 3 nodes for 3 separate application runs

#srun -n 16 -c 8 ./md
srun -n 8 -c 16 ./pv
