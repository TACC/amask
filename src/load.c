#include <string.h>
#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#ifdef AMASK_LOAD
#include "opts.h"
#endif

//include <getopt.h>
//include <iostream>
//include <stdlib.h>
//include <unistd.h>
//include <cstring>
/*
  This routine load the system with integer work.
                               Kent Milfeld
                               2016/10/11
*/
int             myspin(int     );
double      gtod_timer(        );
int     load_cpu_nsec_(int *sec);
void     load_cpu_nsec(int     );
//                          Fortran Version
int load_cpu_nsec_(int *sec){
    load_cpu_nsec(     *sec);
    return 0;
}


void load_cpu_nsec(int sec ){

// Load repeats the executon of an integer reduction (of 10M summations)
// for "sec" seconds.  Initially 10 outer iteration are performed to
// determine the amount of time for an outer iteration.  Then, for the
// remaining time an appropriate number of iterations is determined
// and executed.

int i,j; 
double arg, isum,factor=0.000000000001;
int base_iters=1000000;
long long int iters;

float fsec, test_cost, startup_cost, test_factor, sec_p_base;

double t0,t1;
double tt0,tt1;

//                                New, allow MASKERAID_LOAD_SECONDS to override argument value
char digits[11] = { "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" };
int  slen, knt = 0;

#ifdef AMASK_LOAD
   Maskopts opts;
   if( opts.is_set_by_cmdline() ) sec = opts.get_w();
#else
   const char* senv = getenv("HPCUTILS_LOAD_SECONDS");
   if (senv!=NULL){
      slen=strlen(senv);

      for(j=0;j<slen;j++) for(i=0;i<10;i++)
         if(senv[j] == digits[i] ){knt++;};

    //if(knt == slen){ sec = atoi((void *)senv); }  //if all are ints
      if(knt == slen){ sec = atoi((char *)senv); }  //if all are ints
      if(sec == 0 || knt != slen){
         printf("ERROR: ENV var MASKERAID_LOAD_SECONDS (%s) is invalid;"
                " must be a non-zero int.\n", senv);
         exit(8);
      }

   } // end NULL test
#endif


//                          Determine a base cost to run myspin.
      //warm up
      for(i=0;i<base_iters;i++){ arg=i*factor+0.00002; isum += sin(arg)*cos(arg); }
      if(isum<-1.0e0) printf("%lf\n",isum);   // don't optimize  away myspin
					      //
      t0=gtod_timer();                      // run 10 samples to determine time for base_iter
         for(i=0;i<base_iters;i++){ arg=i*factor; isum += sin(arg)*cos(arg); }
         if(isum<-1.0e0) printf("%lf\n",isum);   // (if on isum) don't optimize  away myspin
      t1=gtod_timer();
      test_cost = t1-t0;
   // printf("test1  : %lf %lf %lf\n",t0,t1,test_cost);

      fsec = (float)sec  - test_cost;

      if(fsec < 0.0e0) return;   // Load will have to be the testing cost!
				 //
      iters = (long long int)(base_iters*fsec/test_cost);

   // printf("ITERS=%ll  fsec=%f  test_cost %f\n",iters,fsec,test_cost);

      for(i=0;i<iters;i++){ arg=i*factor; isum += sin(arg)*cos(arg); }  // @10 0.17
      if(isum<-1.0e0) printf("%lf\n",isum);   // don't optimize  away myspin
}
