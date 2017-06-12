/*   Routine reports hybrid affinity information for MPI processes
   within an OpenMP region.

   Within a parallel region--  
     Rank 0 gathers thread affinities from each rank & reports.

       a.) Within a master region:
           Determine maximum length of node name.
           Gather node names from each rank.
           Determine if there are multiple compute nodes.
             Each mpi process (rank) creates static space:
             for gathering nodes names and length,
             for collecting affinity masks of all threads 
             within the mpi process (omp_proc_mask).
    
       b.) Within the parallel region:
           Determine the mask for the thread (insert into omp_proc_mask)

       c.) Within master region:
           if rank 0
             print mask header
             print masks for rank 0
      
             gather masks from non-rank-0 MPI processes (MPI_Irecv).
             loop over non-rank-0 processes and print masks (for each thread)
           if rank != 0
             pack omp_proc_mask mask into omp_mask_pac and send to rank 0.

             Free static spaces
           Return

     
*/

#include  <stdio.h>
#include    <mpi.h>
#include    <omp.h>
#include  <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include  <ctype.h>
#include  <cstdlib>
#include "opts.h"

                                // basic mask printer-- prints a single row with ncpus number of elements
void print_mask(int hd_prnt, char* name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, int *proc_mask, int tpc, char l);
int boundto(int* nelements_set, int* int_mask);
int get_threads_per_node();


int amask_hybrid(){

                        // General
int i,j,ierr;
int id, rid,tid;
int in_mpi, in_omp;
int thrd, nthrds;
int ncpus, nel_set;

                        // Mask storage
static int ** omp_proc_mask;
static int  * omp_mask_pac;
char *dummy;

                       // MPI specific Variables
int rank, nranks;
MPI_Request *request; 
MPI_Status  *status;

static int   multi_node = 0;
static char *all_names;
static int   max_name_len;
     int   name_len;
     char  proc_name[MPI_MAX_PROCESSOR_NAME];

char l,p;
int  tpc;   // hwthreads/core

   Maskopts opts;
                          // get print_speed fast or slow (f|c);   listing cores or SMT (c|s)
   p = opts.get_p();
   l = opts.get_l();

   tpc=get_threads_per_node();
                                         // In MPI and parallel region ?
   MPI_Initialized(&in_mpi);
   in_omp = omp_in_parallel();
   if(in_mpi == 0){
     printf("ERROR: ***** Must call amask_hybrid() in MPI program. ***** \n");
     exit(1);
   }

                                        // Get rank number & no of ranks via MPI
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &nranks);

   if(in_omp == 0){
     if(rank == 0){
        printf("         ***** When using 1 thread, Intel OpenMP MAY report "
                              "\"not in a parallel region\" (Uh!)***** \n");
        printf("         ***** Each row will only have a rank number (no \"0\" thread_id). \n");
        printf("WARNING: ***** Unspecified results if amask_hybrid "
                              "not called in parallel region of MPI code section. ***** \n");
     }
   }
 
   thrd  =  omp_get_thread_num();        // thread id
   nthrds =  omp_get_num_threads();      // Number of Threads
                                         // Get number of cpus (this gives no. 
                                         // of cpu_ids in /proc/cpuinfo)
   ncpus = (int) sysconf(_SC_NPROCESSORS_ONLN);
 
 
                                  // Working only with MPI processes (masters)
   #pragma omp master
   {
                                  // Get a list of nodes from all ranks.
     MPI_Get_processor_name(proc_name,&name_len);
     MPI_Allreduce(&name_len, &max_name_len, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
     all_names = (char *) malloc(sizeof(int*)*nranks*(max_name_len+1));
     MPI_Gather( proc_name, max_name_len+1 , MPI_CHAR,
                 all_names, max_name_len+1, MPI_CHAR,
                 0, MPI_COMM_WORLD);
  
                                  // If multiple nodes, make muti_node non-zero.
     if(rank == 0){
       for(id=0;id<nranks;id++){
         if( strcmp(&all_names[id*(max_name_len+1)],&all_names[0]) ) multi_node++; }

     }
   
                                  // Create shared storage for masks (only master allocates)
 
     omp_proc_mask =                          (int **) malloc(sizeof(int*)*nthrds);
     for(i=0;i<nthrds;i++) omp_proc_mask[i] = (int * ) malloc(sizeof(int )*ncpus  );
     for(i=0;i<nthrds;i++) for(j=0;j<ncpus;j++) omp_proc_mask[i][j] =0;
 
   }
   #pragma omp barrier

   #pragma omp critical           // (boundto -- may not be thread safe)
     ierr = boundto(&nel_set,omp_proc_mask[thrd]);
   #pragma omp barrier
 
   #pragma omp master
   {
 
     omp_mask_pac = (int *) malloc(sizeof(int)*nranks*nthrds*ncpus);  // need packing space for mpi send/recv

     if(rank == 0){
       request = (MPI_Request *) malloc(sizeof(MPI_Request)*nranks);
       status  = (MPI_Status  *) malloc(sizeof(MPI_Status )*nranks);

       print_mask(1,  dummy,  multi_node,     0, 0,   ncpus, nranks,nthrds, omp_proc_mask[0],tpc,l);  //print header 
       fflush(stdout);

       for(tid=0;tid<nthrds;tid++){
          print_mask(0,  &all_names[tid*(max_name_len+1)], multi_node,  0,tid,   ncpus, nranks,nthrds, omp_proc_mask[tid],tpc,l);
       }
       fflush(stdout);
         
       for(rid=1;rid<nranks;rid++){ // Receive other rank's packed mask arrays
         MPI_Irecv(&omp_mask_pac[rid*nthrds*ncpus], nthrds*ncpus, MPI_INT, rid, 99, MPI_COMM_WORLD, &request[rid-1]);
       } 

       MPI_Waitall(nranks-1,&request[0],&status[0]);

       for(rid=1;rid<nranks;rid++){ // Print for each rank
            for(tid=0;tid<nthrds;tid++){
               print_mask(0,  &all_names[tid*(max_name_len+1)], multi_node,  rid,tid,   ncpus, nranks,nthrds, &omp_mask_pac[rid*nthrds*ncpus + tid*ncpus],tpc,l);
               if(p == 's') ierr=usleep(300000);
            }
       }

       if(nranks*nthrds > 50)
          print_mask(2,  dummy,  multi_node,     0, 0,   ncpus, nranks,nthrds, omp_proc_mask[0],tpc,l);  //print header 

       fflush(stdout);

     }   // end root printing

     else{  //all non-root ranks

                                       // Pack up the ranks' mask arrays (Uh, should have made one array from beginning!) 
          for(   tid=0;tid<nthrds;tid++){
             for( id=0; id<ncpus;  id++)  omp_mask_pac[(tid*ncpus)+id] = omp_proc_mask[tid][id];
             if(p == 's') ierr=usleep(300000);
          }
                                       // Send to root
          MPI_Send(omp_mask_pac, nthrds*ncpus, MPI_INT, 0, 99, MPI_COMM_WORLD);
               

     } // end non-root printing

                                  // Return allocated space

     for(i=0;i<nthrds;i++) free(omp_proc_mask[i]);
                           free(omp_proc_mask);
     free(omp_mask_pac);
     if(rank == 0 ){ free(request); free(status);}
     free(all_names);

   } // end of Master

   #pragma omp barrier            // JIC, what all threads leaving at the same time.

}

void amask_hybrid_(){ (void) amask_hybrid(); }
