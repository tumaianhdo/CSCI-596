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

  strncpy(rmethodname,    argv[1], sizeof(rmethodname));
  strncpy(rmethodparams,  argv[2], sizeof(rmethodparams));

  if (!strcmp(rmethodname,"BP")) {
      read_method = ADIOS_READ_METHOD_BP;
  } else if (!strcmp(rmethodname,"DATASPACES")) {
      read_method = ADIOS_READ_METHOD_DATASPACES;
  } else if (!strcmp(rmethodname,"DIMES")) {
      read_method = ADIOS_READ_METHOD_DIMES;
  } else if (!strcmp(rmethodname,"FLEXPATH")) {
      read_method = ADIOS_READ_METHOD_FLEXPATH;
  } else {
      printf("ERROR: Supported read methods are: BP, DATASPACES, DIMES, FLEXPATH. You selected %s\n", rmethodname);
  }
  

  MPI_Init(&argc,&argv); /* Initialize the MPI environment */
  //MPI_Comm_rank(MPI_COMM_WORLD, &sid);  /* My processor ID */
  MPI_Comm_rank(MPI_COMM_WORLD, &gid);  /* Global processor ID */
  md = 2;
  MPI_Comm_split(MPI_COMM_WORLD, md, gid, &workers);
  MPI_Comm_rank(workers, &sid);
  MPI_Comm_size(workers, &nproc);

  timer_init(&timer_, 1);
  timer_start(&timer_);

  init_params();
  
  if (sid == 0) fpv = fopen("pv.dat","w");

  adios_read_init_method(read_method, workers, rmethodparams);
  f = adios_read_open ("staged.bp", read_method, workers,  
                         ADIOS_LOCKMODE_ALL, 0);
  if (f != NULL) {
   cpu1 = MPI_Wtime();

    for (stepCount=StepAvg; stepCount<=StepLimit; stepCount+=StepAvg) {
        tm_st = timer_read(&timer_);
        //printf("Rank = %d Step = %d\n", sid, stepCount);
        //sprintf(var, "dbuf_%d", sid);

        //varinfo = adios_inq_var (f, var);
        varinfo = adios_inq_var(f, "dbuf");
        if (varinfo) {
            //printf("Rank = %d ndim = %d\n", sid, varinfo->ndim);
            //printf("Rank = %d dims = (", sid);
            datasize = adios_type_size(varinfo->type, varinfo->value);
            for (i = 0; i < varinfo->ndim; i++) {
                datasize *= varinfo->dims[i];
                /*printf ("%d" , varinfo->dims[i]);
                if (i != varinfo->ndim - 1) {
                    printf (",");
                }*/
            }
            //printf (")\n");*/
            data = malloc (datasize);
            decompose (nproc, sid, varinfo->ndim, varinfo->dims, vproc, count, start, &writesize);
            writesize = writesize * adios_type_size(varinfo->type, varinfo->value);
            /*printf("Rank = %d offset = (", sid);
            for (i = 0; i < varinfo->ndim; i++) {
                printf ("%d" , start[i]);
                if (i != varinfo->ndim - 1) {
                    printf (",");
                }
            }
            printf (")\n");
            printf("Rank = %d ldims = (", sid);
            for (i = 0; i < varinfo->ndim; i++) {
                printf ("%d" , count[i]);
                if (i != varinfo->ndim - 1) {
                    printf (",");
                }
            }
            printf (")\n");*/
            sel = adios_selection_boundingbox (varinfo->ndim, start, count);
            adios_schedule_read (f, sel, "dbuf", 0, 1, data);
            adios_perform_reads (f, 1);
            adios_selection_delete (sel);
        }

        adios_free_varinfo(varinfo);

        // Receive # of atoms n to rank gid+1 in MPI_COMM_WORLD
        //MPI_Recv(&n, 1, MPI_INT, gid + 1, 1000, MPI_COMM_WORLD, &status);
        // Receive velocities of n atoms to rank gid+1 in MPI_COMM_WORLD 
        //dbuf = (double *)malloc(sizeof(double) * 3 * n);
        //MPI_Recv(dbuf, 3*n, MPI_DOUBLE, gid + 1, 2000, MPI_COMM_WORLD, &status);
        dbuf = (double *) data;
    
        /*if (stepCount == StepAvg && sid == 0) {
          printf("***************%le %le %le %le %le\n", dbuf[0],dbuf[1],dbuf[2],dbuf[3], dbuf[4]);
        }*/
        n = count[0]/3;
        //n = count[0];
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

        
        adios_release_step (f); // this step is no longer needed to be locked in staging area
        tm_end = timer_read(&timer_);
        tm_diff = tm_end - tm_st;

        MPI_Reduce(&tm_diff, &tm_max, 1, MPI_DOUBLE, MPI_MAX, 0, workers);
        if (sid == 0) {
          printf("TS= %u write MAX time= %lf\n", stepCount, tm_max);
        }
        
        adios_advance_step (f, 0, 0);

    }
    adios_read_close (f);
    cpu = MPI_Wtime() - cpu1;
  }

  adios_read_finalize_method (read_method);
  adios_finalize (sid);

  if (sid == 0) fclose(fpv);

  timer_stop(&timer_);

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
