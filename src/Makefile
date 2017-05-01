CC_SER  =   icpc -g
CC_MPI  = mpicxx -g
CC_OMP  =   icpc -qopenmp -g
CC_HYB  = mpicxx -qopenmp -g
LD_MPI  = mpicxx -g
LD_OMP  =   icpc -qopenmp -g
LD_HYB  = mpicxx -qopenmp -g

AR      = ar

ifeq ($(TACC_SYSTEM),stampede)
   LOAD_X=-xhost
   LOAD_DEF=-DSP1
endif
ifeq ($(TACC_SYSTEM),stampedeknl)
   LOAD_X=-xMIC-AVX512
   LOAD_DEF=-DSPKNL
endif
ifeq ($(TACC_SYSTEM),ls5)
   LOAD_X=-xhost
   LOAD_DEF=-DLS5
endif

#rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod

#                               new load code auto adjusts for units of seconds.
#CFLAGS	= -D _GNU_SOURCE -D LS5 no longer required
#CFLAGS	= -D _GNU_SOURCE -D SP1 no longer required
#CFLAGS	= -D _GNU_SOURCE        Required only for timers, boundto and mapto_procid  added separate rule for timers
 CFLAGS	= 
LDFLAGS	= 

DRIVER_SOURCES = mpi_affinity.c omp_affinity.c hybrid_affinity.c 
DRIVER_OBJECTS = $(DRIVER_SOURCES:.c=.o)

#LIB_SOURCES=boundto.c cmdln_args.c load.c map_to_procid.c print_mask.c timers.c mpi_report_mask.c omp_report_mask.c hybrid_report_mask.c opts.cpp
 LIB_SOURCES=boundto.c              load.c map_to_procid.c print_mask.c timers.c mpi_report_mask.c omp_report_mask.c hybrid_report_mask.c opts.cpp
#LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
 LIB_OBJECTS =boundto.o             load.o map_to_procid.o print_mask.o timers.o mpi_report_mask.o omp_report_mask.o hybrid_report_mask.o opts.o


LIB     = maskeraid.a
EXES	= $(DRIVER_SOURCES:.c=)
#EXES = mpi_affinity omp_affinity hybrid_affinity 

all: $(EXES)

mpi_affinity    :            mpi_affinity.o                       $(LIB)
	$(LD_MPI) $(LDFLAGS_MPI)  mpi_affinity.o    -o mpi_affinity    $(LIB)
	mkdir -p ../bin
	cp -p  mpi_affinity ../bin
omp_affinity    :            omp_affinity.o                       $(LIB)
	$(LD_OMP) $(LDFLAGS_OMP)  omp_affinity.o    -o omp_affinity    $(LIB)
	mkdir -p ../bin
	cp -p  omp_affinity ../bin
hybrid_affinity :            hybrid_affinity.o                    $(LIB)
	$(LD_HYB) $(LDFLAGS_HYB)  hybrid_affinity.o -o hybrid_affinity $(LIB)
	mkdir -p ../bin
	cp -p  hybrid_affinity ../bin


$(LIB) : $(LIB_OBJECTS)
	$(AR) -csr  $@  $^
	mkdir -p ../lib
	cp -p $(LIB) ../lib

mpi_affinity.o    : mpi_affinity.c 
	$(CC_MPI) $(CFLAGS) -c $< -o $@
mpi_report_mask.o : mpi_report_mask.c
	$(CC_MPI) $(CFLAGS) -c $< -o $@

omp_affinity.o    :  omp_affinity.c
	$(CC_OMP) $(CFLAGS) -c $< -o $@
omp_report_mask.o :  omp_report_mask.c
	$(CC_OMP) $(CFLAGS) -c $< -o $@

hybrid_affinity.o    :  hybrid_affinity.c
	$(CC_HYB) $(CFLAGS) -c $< -o $@
hybrid_report_mask.o :  hybrid_report_mask.c
	$(CC_HYB) $(CFLAGS) -c $< -o $@


load.o  : load.c
	$(CC_SER) $(LOAD_X) -DCMASK_LOAD -O3 $(LOAD_DEF)  -c $< -o $@

boundto.o  : boundto.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@
map_to_procid.o  : map_to_procid.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@
timers.o  : timers.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@
opts.o  : opts.cpp
	$(CC_SER)                 -c $< -o $@


%.o : %.c
	$(CC_SER) $(CFLAGS) -c $< -o $@


.PHONY: install clean cleanall
install:
	mkdir -p -v ../bin ../lib
	cp -p $(LIB)  ../lib
	cp -p $(EXES) ../bin

clean :
	echo EXES: $(EXES) 
	echo DRIVER_OBJECTS: $(DRIVER_OBJECTS) 
	echo LIB_OBJECTS: $(LIB_OBJECTS) 
	echo LIB: $(LIB) *.mod
	rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod

cleanall :
	rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod
	rm -fr $(addprefix ../bin/, $(EXES))  ../lib/$(LIB)
	rmdir  ../bin/        ../lib/


#http://stackoverflow.com/questions/15195087/how-to-make-static-library-in-makefile
#other information
