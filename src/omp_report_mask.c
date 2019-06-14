/*   Routine reports OpenMP process affinity information.

        Get thread number and cpus (cpu_ids)
        Create static space (proc_mask) to hold all masks (done in a single region)
        Determine the mask for each thread (insert it in proc_mask)

        print mask header (one thread in single region)
        print mask        (one thread in single region)

        free  spaces
        return

   Removed check: if(omp_get_num_procs() != ncpus){  on 2019-06-14

*/

#include <stdio.h>
#include <omp.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "opts.h"

void print_mask(int hd_prnt, char* name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, int *proc_mask, int tpc, char v);
int boundto(int* nelements_set, int* int_mask);
int get_threads_per_node();

void amask_omp(){

static int ncpus, nthrds;
int        thrd;  //Thread info

int  nel_set;
static int ** proc_mask;
int i,j, ierr;
char *  dummy;

static char v,p;
static int  tpc;   // hwthreads/core

   Maskopts opts;

   thrd   =  omp_get_thread_num();

   #pragma omp single
   {
                          // get print_speed fast or slow (f|c);   listing cores or SMT (c|s)
      p = opts.get_p();
      v = opts.get_v();

      tpc    =  get_threads_per_node();
      nthrds =  omp_get_num_threads();
      ncpus  =  (int) sysconf(_SC_NPROCESSORS_ONLN);
   } 

//  Uh, omp_get_num_procs doesn't return processors on device as advertised.
//  if numactl requests a subset of cpu-ids, it returns the count of bits set.
//  So-- removing this "invalid" check. 6/14/2019 
/*
   if(omp_get_num_procs() != ncpus){
     printf("ERROR: ncpus_by_omp=%d, ncpus_sched=%d\n",omp_get_num_procs(),ncpus);
     exit(1);
   }
*/

   #pragma omp single
   {
     proc_mask =                          (int **) malloc(sizeof(int*)*nthrds);
     for(i=0;i<nthrds;i++) proc_mask[i] = (int * ) malloc(sizeof(int)*ncpus  );
     for(i=0;i<nthrds;i++) for(j=0;j<ncpus;j++) proc_mask[i][j] =0;
   }

   ierr = boundto(&nel_set,proc_mask[thrd]);
   #pragma omp barrier

   #pragma omp single
   {
         print_mask(1,   dummy,  0,     0,0,   ncpus, 1,nthrds, proc_mask[thrd],tpc,v);  //print header
     for(thrd=0;thrd<nthrds;thrd++){
         print_mask(0,   dummy,  0,  0,thrd,   ncpus, 1,nthrds, proc_mask[thrd],tpc,v);
         if(p == 's') ierr=usleep(300000);
     }
     if(nthrds>50)
         print_mask(2,   dummy,  0,     0,0,   ncpus, 1,nthrds, proc_mask[thrd],tpc,v);  //print header
   

     for(i=0;i<nthrds;i++) free( proc_mask[i]);
                           free( proc_mask);
   }

}

void amask_omp_(){ amask_omp(); }
