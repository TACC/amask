//
//   Routine sets an integer bitmask with 1's in the same locations that
//   the numa mask is set.
//
//   Integer mask (int_mask) is supplied to the routine.
//   The number of elements set is returned (in nelements_set).
//   Status is returned.
//
//   C/C++
//    void  boundto(int* bnd2_status,int* nprocs,int* nelements_set, int* int_mask)
//   F90  NOT IMPLEMENTED YET
//    call  boundto( bnd2_status, nprocs, nelements_set, int_mask)
//          integer bnd2_status, nprocs, nelements_set, int_mask(ncpus)
//
//
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <errno.h>

// returns integer mask in int_mask
// returns number of mask elements set in nelement_set

int boundto(int* nelements_set, int* int_mask)
{
          cpu_set_t *mask;
          size_t size;
          int i;
          int nrcpus = 1024;
          int knt = 0;

  realloc:
          mask = CPU_ALLOC(nrcpus);
          size = CPU_ALLOC_SIZE(nrcpus);
          CPU_ZERO_S(size, mask);
          if ( sched_getaffinity(0, size, mask) == -1 ) {
                  CPU_FREE(mask);
                  if (errno == EINVAL &&
                      nrcpus < (1024 << 8)) {
                         nrcpus = nrcpus << 2;
                         goto realloc;
                  }
                  perror("sched_getaffinity");
                  return -1;
          }

          for ( i = 0; i < nrcpus; i++ ) {
                  if ( CPU_ISSET_S(i, size, mask) ) {
                          //printf("CPU %d is set\n", (i));
                          int_mask[i] = 1;
                          knt++;
                  }
          }
          *nelements_set = knt;

          CPU_FREE(mask);

          return 0;
}

 int  boundto_(int *nelements_set, int *int_mask){ 
(int) boundto(      nelements_set,      int_mask);}
