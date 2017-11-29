#!/bin/bash -l
#SBATCH -t 00:01:00
#SBATCH -N 4
#SBATCH -p debug
#SBATCH -L SCRATCH
#SBATCH -C haswell
## On the Cray, you need at least 3 nodes for 3 separate application runs

DATASPACES_DIR=$SCRATCH/software/install/dataspaces

#RUNCMD="srun -n"
RUNCMD="mpirun -np"
#RUNCMD="aprun -n"
SERVER=$DATASPACES_DIR/bin/dataspaces_server

WRITEPROC=8
STAGINGPROC=2
READPROC=4
let "PROCALL=WRITEPROC+READPROC"

# clean-up previous run
rm -f log.* core* conf dataspaces.conf cred
rm staged.bp genarray.bp

# Prepare config file for DataSpaces
echo "## Config file for DataSpaces
ndim = 1
dims = 3000
max_versions = 1
max_readers = 1  
lock_type = 2
" > dataspaces.conf

# Run DataSpaces
echo "-- Start DataSpaces server "$SERVER" on $STAGINGPROC PEs, -s$STAGINGPROC -c$PROCALL"
$RUNCMD $STAGINGPROC $SERVER -s$STAGINGPROC -c$PROCALL &> log.server &

## Give some time for the servers to load and startup
sleep 1s
while [ ! -f conf ]; do
    echo "-- File conf is not yet available from server. Sleep more"
    sleep 1s
done
sleep 3s  # wait server to fill up the conf file

## Export the main server config to the environment
while read line; do
    export set "${line}"
done < conf

echo "-- DataSpaces IDs: P2TNID = $P2TNID   P2TPID = $P2TPID"

# Start STAGE_READ_WRITE
echo "-- Start MD on $WRITEPROC PEs"
$RUNCMD $WRITEPROC ./md >& log.md &

# Start STAGE_READ_WRITE
#echo "-- Start STAGE_READ_WRITE on $READPROC PEs"
#$RUNCMD $READPROC ./stage_read_write staged.bp post.bp DATASPACES "verbose=4;poll_interval=100" MPI "" $READPROC 1 1 >& log.stage_read_write &

# Start STAGE_WRITER
echo "-- Start PV on $READPROC PEs"
$RUNCMD $READPROC ./pv >& log.pv &

echo "-- Wait until all applications exit."
wait
echo "-- All applications exit."
#rm -f conf
