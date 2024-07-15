
# amask Documentation

## Developers and Maintainers
Kent Milfeld  (<mailto:milfeld@tacc.utexas.edu> include amask in subject)

## mkmod Web Sites
* Github:        https://github.com/tacc/amask
* TACC Homepage: https://www.tacc.utexas.edu/research-development/tacc-software/amask

## PURPOSE
The amask tool exposes the process masks (affinity) of an application will have in a parallel environment, so that users can determine the cores/hardware-threads where each process is allowed to run. Amask is designed for use with pure MPI, pure OpenMP, and hybrid (MPI+OpenMP) applications.

## OVERVIEW
The amask executable can be run immediately before an application to obtain the kernel mask that all processes of the application will have (using all the environment affinity settings). The listing (display) consists of an "easy to read" mask consisting of a single row for each process/threads. There are also argumentless API calls for instrumenting codes for directly reporting the masks (affinity) within an application.

## Code Access
To access the amask source code, clone this repository:

    git clone https://github.com/tacc/amask

----------------------------------------------------------------------------

###  USAGE
For applications that use pure OpenMP run `ammask_omp`, for pure MPI run `ammask_mpi`), or for both OpenMP and MPI run `ammask_hybrid`, after setting up the affinity environment for the application (execute it right
before executing the application). Users often experiment with the affinity environment by setting affinity
environment variables and running amask interactively (without running the application).  Below are
basic examples of using amask:

```shell
      $ module load amask
      $ export OMP_NUM_THREADS=4 OMP_PLACES=cores OMP_PROC_BIND=spread
      $ amask_omp      # for OpenMP
      $ pure_omp_app   # not necessary to run app

      $ module load amask
      $ mpirun -np 4 amask_mpi      # for mpi
      $ mpirun -np 4 pure_mpi_app   # not necessary to run app

      $ module load amask
      $ export OMP_NUM_THREADS=4 OMP_PLACES=cores OMP_PROC_BIND=spread
      $ export I_MPI_PIN_DOMAIN=omp:compact   # for intel impi + OpenMP
      $ mpirun -np 4 amask_hybrid
      $ mpirun -np 4 mpi_omp_app   # not necessary to run app

```

### Understanding the output

The display shows a row mask for each process/thread, where the mask (list of available proc-ids
for execution) is determined from the list of digits set in the process row.
Add the digit to the group # above in the header.


Each row in the display is a (kernel) mask for an mpi-process/thread. 
Each character represents a proc-id.
A digit means the proc-id "bit" is set and the process can exececute on the proc-id.
A dash (-) means the bit is not set. The proc-id number is equal
to the digit + column # in the header that bracket the group of 10 number
(the column number is between the bars, e.g  |...20...|).

```script

# ASSUME 16-core system, an 8-core run, with the setting for the app to run on the first 8 cores:
$ module load  amask
$ export OMP_PROC_BIND=close
$ mpirun -np 8 amask_mpi
-------------------------------------------------------------------
     Each row of matrix is an affinity mask. 
     A set mask bit = matrix digit + column group # in |...|
rank |    0    |   10    |
0000 0---------------   
0001 -1--------------
0002 --2-------------
0003 ---3------------    
0004 ----4-----------    digit # + group # in header = proc-id == 14
0005 -----5----------
0006 ------6---------
0006 -------7--------

```

### Features
Use the wait option (-w #) to have the amask executable maintain a load on the processors for # seconds.  When used in with `htop`, this can be a visual check on the position of the process/thread execution.  Also, the view option (-v m) can be used with cores configurations with hyperthreading (multiple hardware threads/core) to observe the mask from a core perspective, as shown here.

Assume a system consists of 2 sockets, each with 28 cores (56 total) and each core is hyperthreaded, making 112 proc-ids. The proc-ids are numbered seqentially on each socket: even numbers on socket 0 (0,2,4,...,54) and odd numbers on socket 1 (1,2,3,...,55).  On each core there are 2 Hardware (HW) threads.  The first HW threads on each core are assigned the usual proc-id with out hyperthreading.  So that on socket 0, the proc-ids (for HW1) are {0,2,4,...,54}, and {1,3,5,...,55} for socket 1.  The 2nd HW thread are assigned by adding 56 to the non-hyperthreaded proc-id, forming the sequence {56,58,60,...,126} and {57,59,61,...,127}, respectively. 

The first display below shows possible occupations of {0,56}, {2,58}, {4,60} for threads 0, 1, and 2 (using -vs to display a single row for the mask).  The second display (the default for hyperthreading, -vm for multiple lines for a mask) shows the columns as essentially core numbers, with HW threads {0,0+56}, {2,2+56}, and {4,4+56}, respectively (as two rows with with == and -- lines).  The latter display (default) is much easier evaluate same-core bit settings.  (With OMP_PROC_BIND=close, the affinity mask below allows a thread to occupy either HW1 or HW2.)


```shell
$ export OMP_NUM_THREADS=2 OMP_PROC_BIND=close
$ amask_omp -vs

     Each row is a mask for a process/thread; each char represents a proc-id.
     A digit means the mask bit is set & process can exec on the proc-id.
     id = digit + column group # of header between the bars (e.g. |...20...|)

      *** Use -vm to generate separate row for each hardware thread.

thrd |    0    |   10    |   20    |   30    |   40    |   50    |   60 ... 110  |
0000 0-------------------------------------------------------6----------...--------
0001 --2-------------------------------------------------------8--------...--------
0002 ----4-------------------------------------------------------0------...--------

$ export OMP_NUM_THREADS=2 OMP_PROC_BIND=close
$ amask_omp 

     Hyperthreading detected: Use -vs to generate a single row for each process.

     Each row is a mask for a process/thread; each char represents a proc-id.
     A digit means the mask bit is set & process can exec on the proc-id.
     id = digit + column group # of header between the bars (e.g. |...20...|)
     For multiple HW threads: add 56 to each additional unlabeled row.

thrd |    0    |   10    |   20    |   30    |   40    |   50    |
0000 0=======================================================
     0-------------------------------------------------------
0001 ==2=====================================================
     --2-----------------------------------------------------
0002 ====4===================================================
     ----4---------------------------------------------------
````


###  API

The amask utility provides an API, its `amask.a` library, for displaying affinity information directly from calls within an application.  Calling the argumentless function `amask_<type>()` (type=`omp`, `mpi` or `hygrid`) in an appropriate point within a parallel context will display the same information reported by the corresponding amask stand-alone executables (amask_omp, amask_mpi and amask_hybrid).  The example code snippets below show appropriate places to call the `amask_<type>` functions.

```C
      // Pure OpenMP Code
      #pragma omp parallel
      amask_omp(); // amask API
      ...


      // Pure MPI Code
      MPI_Init(NULL,NULL);
      amask_mpi(); // amask API
      ...
      MPI_Finalize();

     // Hybrid Code
     MPI_Init(NULL,NULL);
     ...
     #pragma omp parallel
     {
       amask_hybrid(); // amask API
       ...
     }
     ...

    MPI_Finalize();
```

## Installation

### *user* and *system* installation

The Makefile file in src uses the Intel Compiler and IMPI, but it can easily be modified for
other compiler and MPI invironments.  (See also the ifdef for Cori (Cray) and gengua
(generic gcc with MVAPICH2.) Making the Makefile more general is now a TODO item.

It is important to understand that the build is Compiler and MPI specific.  That is, the amask_<type>
commands should be run in the compiler-mpi environment that the application will execute in.
If you have multiple compiler environments (e.g. gcc/Cray/Intel, etc.) and/or MPI environments
(OpenMPI, IMPI, MVAPICH2, etc.) please make a modulefile for each.  Creating a modulefile 
is rather simple with the TACC mkmod utility.
[mkmod documentation]( https://amask.readthedocs.io/en/latest/ )
[github]( https://github.com/tacc/mkmod )





## ChangeLog


## Copyright
(C) 2024 University of Texas at Austin

## MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
