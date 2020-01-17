/*

   Program hybrid_affinity reports the mask for each OMP thread for each MPI process,
   and works for nsec seconds (10). This allows one to inspect occupation through
   utilities like top  (e.g. execute top, then hit the 1 key).

   Uses maskeraid utilities  github.com/TACC/maskeraid
      amask_mpi():     in pure MPI region to report MPI process masks
      amask_hybrid():  in OpenMP parallel region to report thread masks 
      map_to_procid( procid ): sets thread affinity to cpu_id 
                             (see /proc/cpuinfo, or hwloc)
      load_cpu_nsec(nsec):   loads the cpu for nsec (default 10)

  hybrid_affinity.c is a driver for:
     1.) Get line arguments (optional):  help or number of seconds for load
     2.) Start MPI
           Affinity for MPI processes can be reset here.
           amask_mpi() reports MPI process masks
     3.) Start OpenMP parallel region
           amask_hybrid() reports masks for each thread of each
           MPI process.
           
     4.) Set a work load on each thread
     5.) Finish parallel region 
     6.) Stop MPI
                                         Kent Milfeld 12/16/15

  Update to separate require a single call for OpenMP hybrid.
  Uses multi-threaded MPI initialization
                                         Kent Milfeld 2015/07/13
*/

#include <stdio.h>
#include <omp.h>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include "opts.h"


void load_cpu_nsec(int nsec);
void amask_hybrid(void);
int  map_to_procid( int icore);
void amask_mpi(void); 


int main(int argc, char **argv){
int rank, nranks;         // MPI variables.
int nthrds, thrd, procid;  //Thread info
int requested=MPI_THREAD_MULTIPLE, provided;

int nsec = 10;     // Load, default time

int ierr;          // Error number

// cmdln_get_nsec_or_help( &nsec, argc, argv); //optional, get nsec from cmd line
   Maskopts opts(argc,argv);

                  // thread safe init replaces MPI_Init(&argc, &argv);
   MPI_Init_thread(&argc, &argv, requested, &provided);  
   MPI_Comm_size(MPI_COMM_WORLD, &nranks);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   amask_mpi();   // Report JUST MPI process masks

   #pragma omp parallel private(thrd,nthrds,ierr)
   {
      thrd   =   omp_get_thread_num();
      nthrds =   omp_get_num_threads();

  //  procid  =   thrd;                     // set procid to thread number (thrd)
  //  ierr   =   map_to_procid( procid );    // set your own affinity here 

      amask_hybrid();        // Call mask reporter

      load_cpu_nsec( nsec );       // Load up rank process so user can watch top.

   }

   MPI_Finalize();

}
