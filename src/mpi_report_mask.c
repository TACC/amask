/*   Routine reports pure MPI process affinity information.
     Rank 0 gathers thread affinities from each rank & reports.


        Determine maximum length of node name.
        Gather node names from each rank.
        Determine if there are multiple compute nodes.
 
        Determine the mask for each rank

        if rank 0
          gather masks from all  MPI processes (MPI_Irecv).
          print mask header
          print mask 
        endif

        free  spaces
        return

                                                                    Kent Milfeld
                                                                    2016/07/13

*/


#include <stdio.h>
#include <mpi.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "opts.h"

                                  // basic routes
void print_mask(int hd_prnt, char* name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, int *proc_mask, int tpc, char l);
int boundto(int* nelements_set, int* int_mask);
int get_threads_per_node();


int amask_mpi(){

int thrd, nthrds, ncpus;
int rank, nranks;
int multi_node = 0;

char   proc_name[MPI_MAX_PROCESSOR_NAME];
char * all_names;
int    name_len, max_name_len;

                          // General
int i,j,ierr;
int id, nel_set;

                          // Mask storage
int ** proc_mask;
int  * all_masks;

                          // Options
char l,p;
int  tpc;      //hwthreads/core

   Maskopts opts;
                          // get print_speed fast or slow (f|c);   listing cores or SMT (c|s)
   p = opts.get_p();
   l = opts.get_l();

   tpc=get_threads_per_node();

                   // Get number of cpus (this gives no. of cpu_ids in /proc/cpuinfo)
                   // Get rank number & no of ranks via MPI
   ncpus = (int) sysconf(_SC_NPROCESSORS_ONLN);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &nranks);

                   // Create a 2-D array for mask
                   // proc_mask[rank][ncpus] -- for simplicity, size is [ncpus][ncpus]
                   // Thinking ahead for hybrid code.

                   // zero out proc_mask[ncpus][ncpus]  
                   // I could have made proc_mask a single array (proc_mask[ncpus]); but didn't
                   // This is a hold-over from the openmp version that holds everything for all threads.
                   // For MPI I made a continguous collection array (all_masks).
// proc_mask =            malloc(sizeof(int*)*nranks);
   proc_mask =  (int **)  malloc(sizeof(int*)*nranks);
// for(i=0;i<nranks;i++) proc_mask[i] =         malloc(sizeof(int)*ncpus);
   for(i=0;i<nranks;i++) proc_mask[i] =(int *)  malloc(sizeof(int)*ncpus);
   for(i=0;i<nranks;i++) for(j=0;j<ncpus;j++) proc_mask[i][j] =0;
   all_masks =  (int *) malloc(sizeof(int)*nranks*ncpus);

                                // get map for this processor
   ierr=boundto(&nel_set,proc_mask[rank]);

                                // Gather information to rank 0
   MPI_Gather( proc_mask[rank], ncpus, MPI_INT,
               all_masks,       ncpus, MPI_INT,
               0, MPI_COMM_WORLD);


                                // Get a list of nodes from all ranks.
   MPI_Get_processor_name(proc_name,&name_len);
   MPI_Allreduce(&name_len, &max_name_len, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
   all_names = (char *) malloc(sizeof(int*)*nranks*(max_name_len+1));
   MPI_Gather( proc_name, max_name_len+1 , MPI_CHAR,
               all_names, max_name_len+1,  MPI_CHAR,
               0, MPI_COMM_WORLD);

                                // If multiple nodes, make muti_node not equal to 0.
   if(rank == 0){
      for(id=0;id<nranks;id++){
         if( strcmp(&all_names[id*(max_name_len+1)],&all_names[0]) ) multi_node++;
      }
   }

//http://stackoverflow.com/questions/20620421/how-to-send-and-receive-string-using-mpi

                                // Master prints information
   if(rank == 0){
         print_mask(1,  all_names,               multi_node, id,0, ncpus, nranks,1,  all_masks,tpc,l);  //print header
      for(id=0;id<nranks;id++){
         print_mask(0, &all_names[id*(max_name_len+1)], multi_node, id,0, ncpus, nranks,1, &all_masks[id*ncpus],tpc,l);
         if(p == 's') ierr=usleep(1000000);
      }
      if(nranks>25)
         print_mask(2,  all_names,               multi_node, id,0, ncpus, nranks,1,  all_masks,tpc,l);  //print header
   }

                               // Free space
   for(i=0;i<nranks;i++) free( proc_mask[i]);
                         free( proc_mask);
                         free( all_masks);
                         free( all_names);

}

void amask_mpi_(){ (void) amask_mpi(); }
