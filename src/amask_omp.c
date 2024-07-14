/*

   Program omp_affinity reports the mask for each OMP thread, and works
   for nsec seconds (10). This allows one to inspect occupation through
   utilities like top  (e.g. execute top, then hit the 1 key).

   Uses maskeraid utilities  github.com/TACC/maskeraid
      map_to_procid(cpu_id):  will set current thread to cpu_id
      amask_omp():     reports masks of the threads
      load_cpu_nsec(nsec): load the cpu for nsec (default 10)

*/

/*   omp_affinity.c is a driver 
     1.) Get line arguments (optional):  help or number of seconds for load
     2.) Start OpenMP parallel region
           amask_omp() reports masks for each thread
     3.) Set a work load on each thread
     4.) Finish parallel region 
                                         Kent Milfeld 12/16/15

     Added cmd_line argument extraction.
                                                                    Kent Milfeld
                                                                    2016/07/13
*/
#include <stdio.h>
#include <omp.h>
#include "opts.h"

void load_cpu_nsec(int nsec);
void amask_omp();
int  map_to_procid( int icore);


int main(int argc, char *argv[]){

int nthrds, thrd, procid;    //Thread info

int nsec = 5;                // Load, default time

int ierr;                    // Error number

// cmdln_get_nsec_or_help( &nsec, argc, argv); //optional, get nsec from cmd line
   Maskopts opts(argc,argv);

   #pragma omp parallel private(thrd,ierr)
   {
      thrd   =   omp_get_thread_num();
      nthrds =   omp_get_num_threads();

   // procid  =   thrd;                      // set procid to thread number (thrd)
   // ierr   =   map_to_procid( procid );     // set your own affinity here 

      amask_omp();           // Call mask reporter
      load_cpu_nsec( nsec ); // Load up rank process so user can watch top.
   }

}
