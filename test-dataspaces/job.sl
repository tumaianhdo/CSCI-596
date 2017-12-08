#!/bin/bash -l
#SBATCH -N 3
#SBATCH -t 00:10:00
#SBATCH -p debug
#SBATCH -L SCRATCH
#SBATCH -J dataspaces
#SBATCH -C haswell 

STAGING_METHOD=$1

DATASPACES_DIR=$SCRATCH/software/install/dataspaces
RUNCMD="srun -n"
#RUNCMD="mpirun -np"
#RUNCMD="aprun -n"
SERVER=$DATASPACES_DIR/bin/dataspaces_server

WRITEPROC=32
STAGINGPROC=2
READPROC=4
let "PROCALL=WRITEPROC+READPROC"

## Clean up
rm -f conf cred dataspaces.conf log.*

## Create dataspaces configuration file
echo "## Config file for DataSpaces
ndim = 3
dims = 1024,1024,1024
max_versions = 1
max_readers = 1  
lock_type = 2
" > dataspaces.conf

## Run DataSpaces servers
echo "-- Start DataSpaces server "$SERVER" on $STAGINGPROC PEs, -s$STAGINGPROC -c$PROCALL"
$RUNCMD $STAGINGPROC -N 1 -c 32 $SERVER -s$STAGINGPROC -c$PROCALL &> log.server_$STAGING_METHOD &

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

echo "-- Dataspaces Servers initialize successfully"
echo "-- DataSpaces IDs: P2TNID = $P2TNID   P2TPID = $P2TPID"
echo "-- Staging Method: $STAGING_METHOD"

## Run writer application
echo "-- Start WRITER on $WRITEPROC PEs"
$RUNCMD $WRITEPROC -N 1 -c 1 ./test_writer $STAGING_METHOD $WRITEPROC 3 4 4 2 256 256 512 5 1 &> log.writer_$STAGING_METHOD &
#$RUNCMD $WRITEPROC ./test_writer $STAGING_METHOD $WRITEPROC 3 0 0 0 0 0 0 20 1 &> log.writer_$STAGING_METHOD &


## Run reader application
echo "-- Start READER on $READPROC PEs"
$RUNCMD $READPROC -N 1 -c 1 ./test_reader $STAGING_METHOD $READPROC 3 2 2 1 512 512 1024 5 2 &> log.reader_$STAGING_METHOD &
#$RUNCMD $READPROC ./test_reader $STAGING_METHOD $READPROC 3 0 0 0 0 0 0 20 2 &> log.reader_$STAGING_METHOD &

echo "-- Wait until all applications exit."
wait
echo "-- All applications exit."
