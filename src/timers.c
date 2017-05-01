/*
     gtod_timer() returns a double
     gtod_timer() can be called from C/C++ and Fortran
     Uses gettimeofday for timing information.
     Initial time is set to 0.0 --> helps with precision.
     (removes many most significant bits)

     The first call sets time to 0.0; 
     a subsequent call returns the wallclock 
     time since the previous call.

  This C-compiled utility provides an interface
  for C/C++ and Fortran.

  Syntax:
  F90:   double precision val = gtod_timer()
  C/C++  (double)         val = gtod_timer();


     tsc returns rdtsc count as a 64bit integer
     tsc() can be call from C/C++ and Fortran.
     Usually reproducable results with ~10CP
     (latency ~75 CP)

  F90:   integer(kind=8) val = tsc()
  C/C++  (long long)     val = tsc();

                               Kent Milfeld
  Update:

     Revised text above.
                                                                    Kent Milfeld
                                                                    2016/07/13
     
*/
#include <stdlib.h>
#include <sys/time.h>
#include <cstdlib>
#include <sys/time.h>

double gtod_secbase = 0.0E0;

double gtod_timer()
{
   struct timeval tv;
 //void *Tzp=0;
 //struct timezone *Tzp;
   double sec;

   gettimeofday(&tv, NULL);

               /*Always remove the LARGE sec value
                 for improved accuracy  */
   if(gtod_secbase == 0.0E0)
      gtod_secbase = (double)tv.tv_sec;
   sec = (double)tv.tv_sec - gtod_secbase;

   return sec + 1.0E-06*(double)tv.tv_usec;
}
double gtod_timer_()
{
   struct timeval tv;
// void *Tzp=0;
// struct timezone *Tzp;
   double sec;

 //gettimeofday(&tv, Tzp);
   gettimeofday(&tv, NULL);

               /*Always remove the LARGE sec value
                 for improved accuracy  */
   if(gtod_secbase == 0.0E0)
      gtod_secbase = (double)tv.tv_sec;
   sec = (double)tv.tv_sec - gtod_secbase;

   return sec + 1.0E-06*(double)tv.tv_usec;
}


static __inline unsigned long long tsc(void){
  unsigned long a, d;
  unsigned long long d2;

  __asm__ __volatile__ ("rdtsc" : "=a" (a), "=d" (d));

  d2 = d;
  return (unsigned long long) a | (d2 << 32);
};

unsigned long long   tsc_timer(void){ return  tsc(); }
unsigned long long  tsc_timer_(void){ return  tsc(); }
