  #!/bin/bash
  
  ml amask
  
# OMP TEST
  echo "       OMP TEST"
                                     CMD=mpirun 
                                     export OMP_NUM_THREADS=8 NRANKS=2  OMP_PROC_BIND=TRUE CMD=ibrun
  [[ $TACC_SYSTEM == frontera  ]] && export OMP_NUM_THREADS=8 NRANKS=7  OMP_PROC_BIND=TRUE CMD=ibrun
  [[ $TACC_SYSTEM == stampede3 ]] && export OMP_NUM_THREADS=8 NRANKS=12 OMP_PROC_BIND=TRUE CMD=ibrun
  [[ $TACC_SYSTEM == ls6       ]] && export OMP_NUM_THREADS=8 NRANKS=16 OMP_PROC_BIND=TRUE CMD=ibrun
  [[ $TACC_SYSTEM == vista     ]] && export OMP_NUM_THREADS=8 NRANKS=18 OMP_PROC_BIND=TRUE CMD=ibrun
  
  amask_omp
  
# MPI TEST
  echo "       MPI TEST"
  
  $CMD -np $NRANKS amask_mpi
  
  
# HYBRID TEST
  echo "       HYBRID TEST"

  $CMD -np $NRANKS amask_hybrid
