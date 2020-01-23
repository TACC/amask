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

enum  proc_id_ordering { SocSeq_HWT1st, SocEO_Core1st, SocSeq_Core1st, Order_Error };

struct nodeinfo{
    enum proc_id_ordering proc_order;
    int  nproc_ids;
    int  nsockets;
    int  ncores;
    int  TpC;
    int  CpS;
    int  TpC_verified;   // nsockets/ncores/TpC verified
    int  status;   // nsockets/ncores/TpC verified
};

void print_mask(int hd_prnt, char* name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, nodeinfo info, int *proc_mask,  char v);
int boundto(int* nelements_set, int* int_mask);
int get_threads_per_node();
//nodeinfo get_node_info(nodeinfo);
void get_node_info(nodeinfo &info);

void amask_omp(){

nodeinfo info;

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

   #pragma omp master
   {
                          // get print_speed fast or slow (f|c);   listing cores or SMT (c|s)
      p = opts.get_p();
      v = opts.get_v();

      get_node_info(info);
      if(info.status == 0){
          printf("ERROR: lscpu failed.\n");
          fprintf(stderr,"        code file: %s     line: %d\n", __FILE__,__LINE__);
        //ierr=errno;printf("%s\n",strerror(ierr));
      }
      tpc    = info.TpC;
   // tpc    =  get_threads_per_node();
      nthrds =  omp_get_num_threads();
      ncpus  =  (int) sysconf(_SC_NPROCESSORS_ONLN);
   } 
   #pragma omp barrier

//  Uh, omp_get_num_procs doesn't return processors on device as advertised.
//  if numactl requests a subset of cpu-ids, it returns the count of bits set.
//  So-- removing this "invalid" check. 6/14/2019 
/*
   if(omp_get_num_procs() != ncpus){
     printf("ERROR: ncpus_by_omp=%d, ncpus_sched=%d\n",omp_get_num_procs(),ncpus);
     exit(1);
   }
*/

   #pragma omp barrier
   #pragma omp master
   {
//printf("SINGLE1 %d %d %d %d %d %d %d %d\n",  info.proc_order, info.nproc_ids, info.nsockets, info.ncores, info.TpC, info.CpS, info.TpC_verified, info.status);
     proc_mask =                          (int **) malloc(sizeof(int*)*nthrds);
     for(i=0;i<nthrds;i++) proc_mask[i] = (int * ) malloc(sizeof(int)*ncpus  );
     for(i=0;i<nthrds;i++) for(j=0;j<ncpus;j++) proc_mask[i][j] =0;
   }  
   #pragma omp barrier
//                           Syncing memory with a flush here.

   ierr = boundto(&nel_set,proc_mask[thrd]);

   #pragma omp barrier

   #pragma omp master
   {
         print_mask(1,   dummy,  0,     0,0,   ncpus, 1,nthrds, info, proc_mask[thrd],v);  //print header
     for(thrd=0;thrd<nthrds;thrd++){
         print_mask(0,   dummy,  0,  0,thrd,   ncpus, 1,nthrds, info, proc_mask[thrd],v);
         if(p == 's') ierr=usleep(300000);
     }
     if(nthrds>50)
         print_mask(2,   dummy,  0,     0,0,   ncpus, 1,nthrds, info, proc_mask[thrd],v);  //print header
   

     for(i=0;i<nthrds;i++) free( proc_mask[i]);
                           free( proc_mask);
   }

}

void amask_omp_(){ amask_omp(); return; }
