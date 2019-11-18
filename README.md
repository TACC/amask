
[//]: # ( [![Documentation Status](https://readthedocs.org/projects/amask/badge/?version=latest)](https://amask.readthedocs.io/en/latest/) Will use once we do readthedocs for amask )
# amask Documentation

## Developers and Maintainers
Kent Milfeld  (<mailto:milfeld@tacc.utexas.edu> include amask in subject)

## mkmod Web Sites
* Github:        https://github.com/TACC/amask
* TACC Homepage: https://www.tacc.utexas.edu/research-development/tacc-software/amask

## PURPOSE
The amask tool exposes the process masks (affinity) of an application will have in a parallel environment, so that users can determine the cores/hardware-threads where each process is allowed to run. Amask is designed for use with pure MPI, pure OpenMP, and hybrid (MPI+OpenMP) applications.

## OVERVIEW
The amask executable can be run immediately before an application to obtain the kernel mask that all processes of the application will have (using all the environment affinity settings). The listing (display) consists of an "easy to read" mask consisting of a single row for each process. There are also argumentless API calls for instrumenting codes for directly reporting the masks (affinity) within an application.

## Code Access
To access the amask source code, clone this repository:

    git clone https://github.com/tacc/amask

[//]: # ( ## User Documentation Use these when the time comes )
[//]: # ( [Webpages](https://amask.readthedocs.io/en/latest/ ) )
[//]: # ( [PDF](https://media.readthedocs.org/pdf/amask/latest/mkmod.pdf) )
[//]: # ( [Paper]( https://www.ixpug.org/documents/1499277876ISC17_IXPUG_wrkshp_AMASK_milfeld_19_4x3_e.pdf ) )


----------------------------------------------------------------------------

###  USAGE
For applications that use pure OpenMP (type=`omp`), pure MPI (type=`mpi`), or both OpenMP and MPI (type=`hybrid`),
execute the `amask\_<type>` code after setting up the affinity environment for the application (execute it right
before executing the application). Users often experiment with the affinity environment by setting the
environment and running `amask\_<type>` interactively (without running the application).  Below are
basic examples of using amask:

```shell
      $ module load amask
      $ export OMP_NUM_THREADS=4 OMP_PLACES=cores OMP_PROC_BIND=spread
      $ amask_omp
      $ pure_omp_app

      $ module load amask
      $ mpirun -np 4 amask_mpi
      $ mpirun -np 4 pure_mpi_app

      $ module load amask
      $ export OMP_NUM_THREADS=4 OMP_PLACES=cores OMP_PROC_BIND=spread
      $ export I_MPI_PIN_DOMAIN=omp:compact
      $ mpirun -np 4 amask_hybrid
      $ mpirun -np 4 mpi_omp_app

```

### Understanding the output

When SMT (Simultaneous Multi-Threading is not available) the display
shows a mask for each process, where the mask (list of available proc-ids
for execution) is determined from the list of digits set in the process row.
Add the digit to the group # above in the header.

```script

# ASSUME 16-core system, 8-core run, with app running on first 8 cores
# module load  amask
$ mpirun -np 8 amask_mpi
-------------------------------------------------------------------
     Each row of matrix is an affinity mask. 
     A set mask bit = matrix digit + column group # in |...|
rank |    0    |   10    |
0000 0---------------   
0001 -1--------------
0002 --2-------------
0003 ---3------------    
0004 ----4-----------   #digit + group # in header = proc-id
0005 -----5----------
0006 ------6---------
0006 -------7--------

```
###  API

The amask utility provides an API in its `amask.a` library for displaying affinity information directly 
from calls within an application.  Calling the argumentless function `amask_<type>()` (type=`omp`, `mpi` or
`hygrid`) in an appropriate point within a parallel context will display the same information 
reported by the corresponding amask stand-alone executable.  The example code snippets below show appropriate places
to call the `amask\_<type>` functions.

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

It is important to understand that the build is Compiler and MPI specific.  That is, the amask\_<type>
commands should be run in the compiler-mpi environment that the application will execute in.
If you have multiple compiler environments (e.g. gcc/Cray/Intel, etc.) and/or MPI environments
(OpenMPI, IMPI, MVAPICH2, etc.) please make a modulefile for each.  Creating a modulefile 
is rather simple with the TACC mkmod utility.
[mkmod documentation]( https://amask.readthedocs.io/en/latest/ )
[github]( https://github.com/tacc/mkmod )





## ChangeLog


## Copyright
(C) 2019 University of Texas at Austin

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
