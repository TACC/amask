#!/bin/bash
# Use this script to diff files in a WORK area with a BRANCH.
# Check this development src with the branch src

WORK='.'
BRANCH=/home1/00770/milfeld/AMASK/amask_final/src

list='
amask_hybrid.c
amask_mpi.c
amask_omp.c
boundto.c
cmdln_args.c
hybrid_report_mask.c
load.c
Makefile
map_to_procid.c
mpi_report_mask.c
omp_report_mask.c
opts.cpp
opts.h
print_mask.c
timers.c
dodiff.sh
../test.sh
../README
../README.md
../Makefile
'

for i in $list; do
echo "                               $i ================"
diff $WORK/$i $BRANCH/$i
done

