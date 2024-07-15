  #!/bin/bash
  
  ml amask
  
# OMP TEST
                                     export OMP_NUM_THREADS=4 OMP_PROC_BIND=TRUE
  [[ $TACC_SYSTEM == frontera  ]] && export OMP_NUM_THREADS=7 OMP_PROC_BIND=TRUE
  [[ $TACC_SYSTEM == ls6       ]] && export OMP_NUM_THREADS=7 OMP_PROC_BIND=TRUE
  [[ $TACC_SYSTEM == vista     ]] && export OMP_NUM_THREADS=7 OMP_PROC_BIND=TRUE
  [[ $TACC_SYSTEM == stampede3 ]] && export OMP_NUM_THREADS=7 OMP_PROC_BIND=TRUE
  
  amask_omp
  
# MPI TEST
                                     CMD=mpirun NRANKS=8
  [[ $TACC_SYSTEM == frontera  ]] && CMD=ibrun  NRANKS=8
  
  $CMD -np $NRANKS amask_mpi
  
  
# HYBRID TEST

                                     CMD=mpirun NRANKS=8
  [[ $TACC_SYSTEM == frontera  ]] && CMD=ibrun  NRANKS=8
  
  $CMD -np $NRANKS amask_hybrid
