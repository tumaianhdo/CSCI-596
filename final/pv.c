/*----------------------------------------------------------------------
Program pmd.c performs parallel molecular-dynamics for Lennard-Jones 
systems using the Message Passing Interface (MPI) standard.
----------------------------------------------------------------------*/
#include "pv.h"


/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
/*--------------------------------------------------------------------*/
  int i, a;
  double cpu1;

  MPI_Init(&argc,&argv); /* Initialize the MPI environment */
  //MPI_Comm_rank(MPI_COMM_WORLD, &sid);  /* My processor ID */
  MPI_Comm_rank(MPI_COMM_WORLD, &gid);  /* Global processor ID */
  md = 2;
  MPI_Comm_split(MPI_COMM_WORLD, md, gid, &workers);
  MPI_Comm_rank(workers, &sid);

  init_params();
  
  if (sid == 0) fpv = fopen("pv.dat","w");

  cpu1 = MPI_Wtime();
  for (stepCount=1; stepCount<=StepLimit; stepCount++) {
    if (stepCount % StepAvg == 0) {
      // Receive # of atoms n to rank gid+1 in MPI_COMM_WORLD
      MPI_Recv(&n, 1, MPI_INT, gid + 1, 1000, MPI_COMM_WORLD, &status);
      // Receive velocities of n atoms to rank gid+1 in MPI_COMM_WORLD 
      dbuf = (double *)malloc(sizeof(double) * 3 * n);
      MPI_Recv(dbuf, 3*n, MPI_DOUBLE, gid + 1, 2000, MPI_COMM_WORLD, &status);
      rv = (double **) malloc(sizeof(double*) * n);
      for (i = 0; i < n; i++) {
        rv[i] = (double *) malloc(sizeof(double) * 3);
        for (a = 0; a < 3; a++)
          rv[i][a] = dbuf[3*i+a];
      }
      calc_pv();

      //Free memory 
      for (i = 0; i < n; i++) {
        free(rv[i]);
      }
      free(rv);
      free(dbuf);
    }
  }
  cpu = MPI_Wtime() - cpu1;
  if (sid == 0)
    fclose(fpv);

  MPI_Finalize(); /* Clean up the MPI environment */
  return 0;
}

/*--------------------------------------------------------------------*/
void init_params() {
/*----------------------------------------------------------------------
Initializes parameters.
----------------------------------------------------------------------*/
  FILE *fp;

  /* Read control parameters */
  fp = fopen("pmd.in","r");
  fscanf(fp, "%*[^\n]\n", NULL);
  fscanf(fp, "%*[^\n]\n", NULL);
  fscanf(fp, "%*[^\n]\n", NULL);
  fscanf(fp, "%*[^\n]\n", NULL);
  fscanf(fp,"%d",&StepLimit);
  fscanf(fp,"%d",&StepAvg);
  fclose(fp);

  printf("Step Limit = %d\n", StepLimit);
  printf("Step Average = %d\n", StepAvg);
}

/*--------------------------------------------------------------------*/
void calc_pv() {
/*----------------------------------------------------------------------
Calculate pv
----------------------------------------------------------------------*/

  double lpv[NBIN],pv[NBIN],dv,v;
  int i;

  dv = VMAX/NBIN;  // Bin size
  for (i=0; i<NBIN; i++) lpv[i] = 0.0; // Reset local histogram
  for (i=0; i<n; i++) {
    v = sqrt(pow(rv[i][0],2)+pow(rv[i][1],2)+pow(rv[i][2],2));
    lpv[v/dv < NBIN ? (int)(v/dv) : NBIN-1] += 1.0;
  }
  MPI_Allreduce(lpv,pv,NBIN,MPI_DOUBLE,MPI_SUM,workers);
  MPI_Allreduce(&n,&nglob,1,MPI_INT,MPI_SUM,workers);
  for (i=0; i<NBIN; i++) pv[i] /= (dv*nglob);  // Normalization
  if (sid == 0) {
    for (i=0; i<NBIN; i++) fprintf(fpv,"%le %le\n",i*dv,pv[i]);
    fprintf(fpv,"\n");
  }
}