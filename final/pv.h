/*----------------------------------------------------------------------
pmd.h is an include file for a parallel MD program, pmd.c.
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

/* Constants------------------------------------------------------------  
VMAX: Max. velocity value to construct a velocity histogram
NBIN: # of bins in the histogram
----------------------------------------------------------------------*/

#define VMAX 5.0  // Max. velocity value to construct a velocity histogram
#define NBIN 100  // # of bins in the histogram

/* Variables------------------------------------------------------------

n = # of resident atoms in this processor.
nb = # of copied boundary atoms from neighbors.
nglob = Total # of atoms summed over processors.
rv: rv[i][0|1|2] is the x|y|z velocity of atom i (including 
dbuf: Buffer for sending double-precision data
sid = Sequential processor ID.
status: Returned by MPI message-passing routines.
cpu: Elapsed wall-clock time in seconds.
comt: Communication time in seconds.
stepCount = Current time step.

----------------------------------------------------------------------*/

int n,nb,nglob;
double **rv;
double *dbuf;
int sid;
MPI_Status status;
MPI_Comm workers;
int gid, sid, md;
FILE *fpv;
double cpu,comt;
int stepCount;

/* Input data-----------------------------------------------------------

Control data: pmd.in.
----------------------------------------------------------------------*/

int StepLimit;      /* Number of time steps to be simulated */
int StepAvg;        /* Reporting interval for statistical data */

/* Functions & function prototypes------------------------------------*/

void calc_pv();
void init_params();

/*--------------------------------------------------------------------*/

