#!/bin/bash -l
#SBATCH -t 00:30:00
#SBATCH -N 2
#SBATCH -p debug
#SBATCH -L SCRATCH
#SBATCH -C haswell
## On the Cray, you need at least 3 nodes for 3 separate application runs

#RUNCMD="srun -n"
RUNCMD="mpirun -np"
#RUNCMD="aprun -n"

WRITEPROC=8
READPROC=4

# Start STAGE_READ_WRITE
echo "-- Start MD on $WRITEPROC PEs"
$RUNCMD $WRITEPROC ./md MPI "verbose=3" >& log.md_no_staging

# Start STAGE_WRITER
echo "-- Start PV on $READPROC PEs"
$RUNCMD $READPROC ./pv BP "verbose=3" >& log.pv_no_staging