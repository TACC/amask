# Find the type of compiler and mpi
#     Cannot depend on environment variables MPI_ROOT, MPI_HOME, MPICH_HOME or I_MPI_ROOT
#     Some sites leave variables such as I_MPI_ROOT set, when changing from impi to openmpi
#     Will use `which mpi`, to determine mpi type (very often it can be found in the path)
#     Will use the compiler from `mpicc -show` (first string is the cc ompiler used)
#

# 
# New, general logic for Compiler and MPI determination    Kent Milfeld 2020/1/9
# It isn't necessary to know which MPI type is used.  
# The MPI logic is for future use or for other makefiles.
#
 AR       = ar

 DB       = -g

 CXX_MPI  = mpicxx
 CXX_HYB  = mpicxx

 LD_MPI   = mpicxx
 LD_HYB   = mpicxx

#
#                                       Compiler/MPI logic by Kent Milfeld 2020/1/9

#  Determine if Cray machine (uses ftn, CC and cc for serial and mpi code or non-Cray

 CC_status     := $(shell CC -V        >/dev/null 2>&1 || echo 1)
 mpicxx_status := $(shell mpicxx -show >/dev/null 2>&1 || echo 1)

ifneq ($(mpicxx_status),1) #ON a non-Cray or Cray Jail Broke machine

   #                                       Extract MPI type from mpicxx path
   #                                       Generally works better han Env. Vars
   mpicxx_path := $(shell which  mpicxx)
   mpi_type = unknown
   
   ifneq (,$(findstring spectrum_mpi,$(mpicxx_path)))
     mpi_type = spectrum_mpi
     #          uses: mpif90 mpif77 mpifort    mpicc     mpic++ mpiCC mpicxx  -->opal_wrapper Spec_OMPI
     #          uses:            mpipgifort  mpipgcc  mpipgic++               -->opal_wrapper Spec_OMPI
     #          uses: mpixlf                  mpixlc                  mpixlC  -->opal_wrapper Spec_OMPI
   endif
   
   ifneq (,$(findstring impi,$(mpicxx_path)))
     mpi_type = impi
   else                                     #impi path may have compilers_and_libraries instead of impi
      ifneq (,$(findstring compilers_and_libraries,$(mpicxx_path)))
        mpi_type = impi
      endif
     #          uses: mpif90 mpif77 ->mpiifort  mpicc ->mpiicc mpicxx-> mpiicpc  IMPI
   endif
   
   ifneq (,$(findstring mvapich2,$(mpicxx_path)))
     mpi_type = mvapich2
     #          uses: mpif90 mpif77 ->mpifort  mpicc mpic++-> mpicxx  MV2
   endif
   
   ifneq (,$(findstring openmpi,$(mpicxx_path)))
     mpi_type = openmpi
     #          uses: mpif90 mpif77 mpifort  mpicc mpic++ mpiCC mpicxx  -->opal_wrapper OMPI
   endif
   
   ifneq (,$(findstring cray_mpich,$(mpicxx_path)))
     mpi_type = cray_mpich
     #          uses: mpif90, mpif77, mpicc, mpicxx
   endif
endif

ifneq ($(CC_status),1)   #ON a Cray  machine
     mpi_type = cray_mpich
endif


ifneq ($(mpicxx_status),1) #ON a non-Cray or Cray Jail Broke machine
   #                                       Extract compiler family from mpicxx command
   #                                       first argument (command) may be a full path
   mpicxx_show := $(shell mpicxx -show | awk '{print $1}' )
   mpicc_compiler = unknown
   
   ifneq (,$(findstring xlC_r,$(mpicxx_show)))
     CXX      =  xlC_r
     OMP_FLAG = -fopenmp
     ARCH     = -Dppc64le

     #          -qsmp=omp  works too, -fopenmp is actually an acceptance of gcc syntax
     #          usage:
     #          mpif90 mpif77 mpifort    mpicc     mpic++ mpiCC mpicxx  -->opal_wrapper Spec_OMPI
     #          xlf_r                    xlc_r                  xlC_r
   endif
   
   ifneq (,$(findstring icpc,$(mpicxx_show)))
     CXX      =  icpc
     OMP_FLAG = -fopenmp
     ARCH     = -DX86_64

     # usage:
     # mpif90 mpif77 ->mpifort  mpicc mpic++-> mpicxx  MV2
     #                   ifort    icc          icpc
   endif
   
   ifneq (,$(findstring pgc++,$(mpicxx_show)))
     CXX      =  pgc++
     OMP_FLAG = -mp
     ARCH     = 
   
     # usage:
     # mpif90 mpif77 mpifort  mpicc mpic++ mpiCC mpicxx  -->opal_wrapper OMPI
     #             pgfortran   pgcc               pgc++
   endif
   
   ifneq (,$(findstring g++,$(mpixx_show)))
     CXX      =  g++
     OMP_FLAG = -fopenmp
     ARCH     = 

     # usage:
     # mpif90 mpif77 ->mpifort  mpicc mpic++-> mpicxx  MV2
     #                gfortran    gcc            g++ 
   endif
   
endif

ifneq ($(CC_status),1)   #ON a Cray  machine
   CXX   =   CC
   OMP_FLAG = -fopenmp
#  TODO
#  need to test CC -craype-verbose to determine compiler here, so as to 
#  apply -homp, -fopenmp, -qopenmp, -mp or -openmp
#  -fopenmp will work for icpc and g++
endif

 LD_OMP=$(CXX)
 CFLAGS	= 
LDFLAGS	= 

DRIVER_SOURCES = amask_mpi.cpp amask_omp.cpp amask_hybrid.cpp
DRIVER_OBJECTS = $(DRIVER_SOURCES:.cpp=.o)

LIB_SOURCES=boundto.cpp load.cpp map_to_procid.cpp print_mask.cpp timers.cpp mpi_report_mask.cpp omp_report_mask.cpp hybrid_report_mask.cpp opts.cpp
LIB_OBJECTS=boundto.o   load.o   map_to_procid.o   print_mask.o   timers.o   mpi_report_mask.o   omp_report_mask.o   hybrid_report_mask.o   opts.o

#SOURCES = $(wildcard *.cpp)
#OBJS = $(OBJS:.cpp=o)


LIB     = amask.a
EXES	= $(DRIVER_SOURCES:.cpp=)
#EXES = amask_mpi amask_omp amask_hybrid 

all: $(EXES)

amask_mpi    :                         amask_mpi.o                  $(LIB)
	$(LD_MPI)  $(DB)               amask_mpi.o  -o amask_mpi    $(LIB)
	mkdir   -p ../bin
	cp      -p  amask_mpi ../bin
amask_omp    :                         amask_omp.o                  $(LIB)
	$(LD_OMP)    $(DB) $(OMP_FLAG) amask_omp.o  -o amask_omp    $(LIB)
	mkdir   -p ../bin
	cp      -p  amask_omp ../bin
amask_hybrid :                       amask_hybrid.o                 $(LIB)
	$(LD_HYB)  $(DB) $(OMP_FLAG) amask_hybrid.o -o amask_hybrid $(LIB)
	mkdir   -p ../bin
	cp      -p  amask_hybrid ../bin


$(LIB) : $(LIB_OBJECTS)
	$(AR)   -csr  $@  $^
	mkdir   -p ../lib
	cp      -p $(LIB) ../lib

#                              Pure MPI Compilations
amask_mpi.o: amask_mpi.cpp
	$(CXX_MPI)  $(DB) $(CFLAGS)            -c $< -o $@

mpi_report_mask.o: mpi_report_mask.cpp
	$(CXX_MPI)  $(DB) $(CFLAGS)            -c $< -o $@

#                                Hybrid Compilations
amask_hybrid.o: amask_hybrid.cpp
	$(CXX_HYB)  $(DB) $(CFLAGS) $(OMP_FLAG) -c $< -o $@

hybrid_report_mask.o: hybrid_report_mask.cpp
	$(CXX_HYB)  $(DB) $(CFLAGS) $(OMP_FLAG) -c $< -o $@

#                              Pure OMP Compilations
amask_omp.o: amask_omp.cpp
	$(CXX)    $(DB) $(CFLAGS) $(OMP_FLAG)  -c $< -o $@

omp_report_mask.o: omp_report_mask.cpp
	$(CXX)    $(DB) $(CFLAGS) $(OMP_FLAG)  -c $< -o $@



#                                Serial Compilations
load.o: load.cpp
	$(CXX)    $(DB) -DCMASK_LOAD -O3       -c $< -o $@

boundto.o: boundto.cpp
	$(CXX)    $(DB) -D_GNU_SOURCE          -c $< -o $@

map_to_procid.o: map_to_procid.cpp
	$(CXX)    $(DB) -D_GNU_SOURCE         -c $< -o $@

timers.o: timers.cpp
	$(CXX)    $(DB) -D_GNU_SOURCE $(ARCH)  -c $< -o $@

opts.o: opts.cpp
	$(CXX)    $(DB)                        -c $< -o $@


%.o: %.cpp
	$(CXX)    $(DB) $(CFLAGS)              -c $< -o $@


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
