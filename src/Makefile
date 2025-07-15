
ifeq ($(PREFIX),)
    PREFIX := ..
endif

LOAD_OPT="-O3"

OMP_OPTION=-fopenmp
CC=icpc  
MPICC=cxx     #intel   CC=g++ GNU  CC=nv++ NVIDIA
DIAG_OFF=

ifeq ($(TACC_SYSTEM),$(filter $(TACC_SYSTEM),frontera ls6))
   CC=icpc
   OMP_OPTION=-qopenmp
endif
    
ifeq ($(TACC_SYSTEM),stampede3)
   CC=icpx
   OMP_OPTION=-qopenmp
endif

ifeq ($(TACC_SYSTEM),vista)
   CC=nvc++
   OMP_OPTION=-fopenmp
#   DIAG_OFF=--diag_suppress declared_but_not_referenced
    DIAG_OFF=--diag_suppress declared_but_not_referenced,set_but_not_used,implicit_return_from_non_void_function,used_before_set
    LOAD_OPT=""
endif

ifeq ($(CHPC_HOST),lengau)
CC=c++
endif

# Intel Compilers (IMPI or MVAPICH2)
CC_SER  =    ${CC}
CC_MPI  = mpi$(MPICC)
CC_OMP  =    ${CC}    $(OMP_OPTION)
CC_HYB  = mpi$(MPICC) $(OMP_OPTION)

LD_MPI  = mpi$(MPICC)
LD_OMP  =    ${CC}    $(OMP_OPTION)
LD_HYB  = mpi$(MPICC)  $(OMP_OPTION)

D_LSCPU=
ifeq ($(shell lscpu >& /dev/null && echo 1 || echo 0), 1)
D_LSCPU=-DHAS_LSCPU
endif

#CHPC South Africa:  Intel Compiler (OpenMPI)
#CHPC_HOST =lengau

AR      = ar

ifeq ($(NERSC_HOST),cori)
   OMP_OPTION=-qopenmp
   CC_SER  = CC
   CC_MPI  = CC
   CC_OMP  = CC ${OMP_OPTION}
   CC_HYB  = CC ${OMP_OPTION}

   LD_MPI  = CC 
   LD_OMP  = CC -fopenmp
   LD_HYB  = CC -fopenmp
endif


#rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod

#                               new load code auto adjusts for units of seconds.
#CFLAGS	= -D _GNU_SOURCE -D LS5 no longer required
#CFLAGS	= -D _GNU_SOURCE -D SP1 no longer required
#CFLAGS	= -D _GNU_SOURCE        Required only for timers, boundto and mapto_procid  added separate rule for timers
 CFLAGS	= 
LDFLAGS	= 

DRIVER_SOURCES = amask_mpi.c amask_omp.c amask_hybrid.c 
DRIVER_OBJECTS = $(DRIVER_SOURCES:.c=.o)

#LIB_SOURCES=boundto.c cmdln_args.c load.c map_to_procid.c print_mask.c timers.c mpi_report_mask.c omp_report_mask.c hybrid_report_mask.c opts.cpp
 LIB_SOURCES=boundto.c              load.c map_to_procid.c print_mask.c timers.c mpi_report_mask.c omp_report_mask.c hybrid_report_mask.c opts.cpp
#LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
 LIB_OBJECTS =boundto.o             load.o map_to_procid.o print_mask.o timers.o mpi_report_mask.o omp_report_mask.o hybrid_report_mask.o opts.o


LIB     = amask.a
EXES	= $(DRIVER_SOURCES:.c=)
#EXES = amask_mpi amask_omp amask_hybrid 

all: $(EXES)

amask_mpi    :            amask_mpi.o $(LIB)
	$(LD_MPI) $(LDFLAGS_MPI)  amask_mpi.o    -o amask_mpi   $(LIB) $(DIAG_OFF)
	mkdir -p ../bin
	cp -p  amask_mpi ../bin
amask_omp    :            amask_omp.o $(LIB)
	$(LD_OMP) $(LDFLAGS_OMP)  amask_omp.o    -o amask_omp   $(LIB) $(DIAG_OFF)
	mkdir -p ../bin
	cp -p  amask_omp ../bin
amask_hybrid :            amask_hybrid.o $(LIB)
	$(LD_HYB) $(LDFLAGS_HYB)  amask_hybrid.o -o amask_hybrid $(LIB) $(DIAG_OFF)
	mkdir -p ../bin
	cp -p  amask_hybrid ../bin


$(LIB) : $(LIB_OBJECTS)
	$(AR) -csr  $@  $^
	mkdir -p ../lib
	cp -p $(LIB) ../lib

amask_mpi.o    : amask_mpi.c 
	$(CC_MPI) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)
mpi_report_mask.o : mpi_report_mask.c
	$(CC_MPI) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)

amask_omp.o    :  amask_omp.c
	$(CC_OMP) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)
omp_report_mask.o :  omp_report_mask.c
	$(CC_OMP) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)

amask_hybrid.o    :  amask_hybrid.c
	$(CC_HYB) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)
hybrid_report_mask.o :  hybrid_report_mask.c
	$(CC_HYB) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)


load.o  : load.c
	$(CC_SER) $(LOAD_X) -DAMASK_LOAD $(LOAD_OPT)  -c $< -o $@ $(DIAG_OFF)

boundto.o  : boundto.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@ $(DIAG_OFF)
map_to_procid.o  : map_to_procid.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@ $(DIAG_OFF)
timers.o  : timers.c
	$(CC_SER)  -D _GNU_SOURCE -c $< -o $@ $(DIAG_OFF)
opts.o  : opts.cpp
	$(CC_SER)                 -c $< -o $@ $(DIAG_OFF)
print_mask.o  : print_mask.c
	$(CC_SER) $(D_LSCPU) -std=c++11 -c $< -o $@ $(DIAG_OFF)


%.o : %.c
	$(CC_SER) $(CFLAGS) -c $< -o $@ $(DIAG_OFF)


.PHONY: install clean cleanall

install :
	mkdir -p -v             $(PREFIX)/bin $(PREFIX)/lib $(PREFIX)/doc
	cp -p $(LIB)            $(PREFIX)/lib
	cp -p $(EXES)           $(PREFIX)/bin
	cp -pr ../documents/*   $(PREFIX)/doc

clean :
	echo EXES: $(EXES) 
	echo DRIVER_OBJECTS: $(DRIVER_OBJECTS) 
	echo LIB_OBJECTS: $(LIB_OBJECTS) 
	echo LIB: $(LIB) *.mod
	rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod

cleanall :
	rm -fr $(EXES) $(DRIVER_OBJECTS) $(LIB_OBJECTS) $(LIB) *.mod
	rm -fr $(PREFIX)/bin  $(PREFIX)/lib $(PREFIX)/doc


#http://stackoverflow.com/questions/15195087/how-to-make-static-library-in-makefile
#other information
