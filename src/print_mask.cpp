/* Routine print mask with node-name, rank #, and thread number prefix when appropriate.

   UPDATE:
     Now allows up to 999 for max cpu_id value (should hold for a few years)
                                                                    Kent Milfeld
                                                                    2016/07/13
     Intel 18.0.2 and 19.x.x are not allowed to use popen.  
     It is broken for 18.  19.x.x gives a warning about using
     popen in parallel region-- but it is called from a single
     region, and that should be OK.  GCC does not complain.         Kent Milfeld
                                                                    2019-06-14
     Conditional compilation changed to include all 18 
     compilers above update 1:   from __INTEL_COMPILER_UPDATE >= 2  2019-07-22
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

enum  proc_id_ordering { SocSeq_HWT1st, SocEO_Core1st, SocSeq_Core1st, Order_Error };

struct nodeinfo{
    enum proc_id_ordering proc_order;
    int  nproc_ids;
    int  nsockets;
    int  ncores;
    int  TpC;
    int  CpS;
    int  TpC_verified;   // nsockets/ncores/TpC verified
    int  status;   // nsockets/ncores/TpC verified
};
const char * C[] = {
    "\033[102m",
    "\033[103m",
    "\033[105m",
    "\033[106m",
    "\033[107m",
    "\033[101m",
    "\033[100m",
    "\033[104m"
};
const char RESET[] = {"\033[0m"};



#define MAX_NAME 4096
                      //!!! If you change NODE_SIZE from 20, change %20s in 2 multi-node format statements.
#define NODE_SIZE  20

int get_threads_per_node();   
int TpC_from_lscpu_tmpfile(void);
int TpC_from_lscpu_popen(void);


void print_mask(int hd_prnt, char *name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, nodeinfo info, int *icpus, char l){ 

int tpc;
int i, ierr, lsdigit, n10, n100;
char node_header[MAX_NAME];

int ic, id, it;

int cores, cnt, wrap_cont = 0;
char no_occ;
int ii, force_long = 0;   // pass this in later
                               // space arrays
char   space_1[] =" ";
char   space_5[] ="     ";
char   space_10[]="          ";
char * spaces;

//                              prints mask acquired for particular rank and thread
//                              ncpus = number of cpu_id's on a node
//                              nranks, nthreads = number of ranks and thread on node
//                              icpus is an int array for mask: 1=occupied 0= not occ.
//                              print/not print header  hd_prnt = 1/0
//    Print Format                          
//    Header:  rank thrd  |        |   10    |   20    | ...
//    printed:   1   1    --234---------------123-------
//
//    icpus               001110000000000000001110000000

//    meaning: rank 1, thread 1 mask allows occupation on cpu_id 2,3,4 and 21,22,23
//    If nranks = 1, rank   number is not printed.
//    If nthrds = 1, thread number is not printed.

//    See FORTRAN INTERFACE at end
//void print_mask(int hd_prnt, char *name, int multi_node, int rank, int thrd, int ncpus, int nranks, int nthrds, nodeinfo info, int *icpus, char l){ 

   int color=1;

   int k, is, ic_socket, sockets, cores_per_socket;
   char str[6096];    // for 1000 cores on node: need 1K bytes
                      // + wrap_cont + tpc*6B (C[i]=6B) + 4B (RESET)
                      // This should line size out to 2050, I hope :)

  tpc=info.TpC;
  sockets=info.nsockets;

  if( l != 'k') {
      cores=ncpus/tpc;
  }
  else{
      tpc=1;
      cores=ncpus;  // effectively make hw_threads/core = 1
  }
//printf("TOP OF FUNCTION: (cores:%d ) ncpus=%d  tpc=%d,  info.TpC=%d  info.\n", cores,ncpus,tpc,info.TpC);
//printf("*** %d %d %d %d %d %d %d %d\n",  info.proc_order, info.nproc_ids, info.nsockets, info.ncores, info.TpC, info.CpS, info.TpC_verified, info.status);
//tpc=4;

                          // If node names are different, then list them in output
  if(multi_node && hd_prnt){

     for(i=0; i<=NODE_SIZE;i++)      node_header[i]           = ' ';
     node_header[NODE_SIZE-4] = 'n'; node_header[NODE_SIZE-3] = 'o'; 
     node_header[NODE_SIZE-2] = 'd'; node_header[NODE_SIZE-1] = 'e';
     node_header[NODE_SIZE+1] = ' '; node_header[NODE_SIZE+2] = '\0';

  }

  if(hd_prnt){   

// Do this the easy way!
//                              HEADER (ranks and threads part)
//                              First part of Header, rank/thrd title
//
    if(nranks == 1 &&  nthrds == 1 ){ spaces=space_1; }         // serial
    if(nranks != 1 &&  nthrds == 1 ){ spaces=space_5; }         // mpi only
    if(nranks == 1 &&  nthrds != 1 ){ spaces=space_5;}         // threads only
    if(nranks != 1 &&  nthrds != 1 ){ spaces=space_10;}         // hybrid

    if( l == 's') {
       if(hd_prnt == 1){             //spaces:  5 for rank/thrd and 10 for rank thrd
                                     printf("\n%sEach row of matrix is an affinity mask.\n",spaces);
                                     printf("%sA set mask bit = matrix digit + column group # in |...|\n\n",spaces);
                       }
    }
    else{
       if(hd_prnt == 1){             //spaces:  5 for rank/thrd and 10 for rank thrd
                                     printf("\n%sEach row of matrix is a mask for a Hardware Thread (hwt).\n",spaces);
                                     printf("%sCORE ID  = matrix digit + column group # in |...|\n",spaces);
                                     printf("%sA set mask bit (proc-id) = core id + add %d to each additional row.\n\n",spaces, cores);
                       }
 
    }

  //if(multi_node){               printf(" %20s",node_header); wrap_cont += 20; } // Multi-node
    if(multi_node){               printf(" %20s",node_header); wrap_cont += 20; } // Multi-node

    if(nranks == 1 &&  nthrds == 1 ){ printf(" ");          wrap_cont += 1;  spaces=space_1; }         // serial
    if(nranks != 1 &&  nthrds == 1 ){ printf("rank ");      wrap_cont += 5;  spaces=space_5; }         // mpi only
    if(nranks == 1 &&  nthrds != 1 ){ printf("thrd ");      wrap_cont += 5;  spaces=space_5;}         // threads only
    if(nranks != 1 &&  nthrds != 1 ){ printf("rank thrd "); wrap_cont += 10; spaces=space_5;}         // hybrid


//                              HEADER (Groups of 10's)
//                              Print out Header
                                printf("|    0    |");                                
     cnt = ( tpc > 1 && ! force_long )? cores : ncpus;
     if(cnt < 100){
//                                                          # of header groups < 100 
        n10  = ( (int) ceilf( (float)cnt/10 ) );
        for(i = 1; i<n10;  i++) printf("   %2d    |",i*10);
     }else{
//                                                           # of header groups > 100
        n100 = ( (int) ceilf( (float)cnt/10 ) ) - 10 ;
        for(i = 1; i<10  ; i++) printf("   %2d    |",i*10    );
        for(i = 0; i<n100; i++) printf("   %3d   |", i*10+100);
     }
                                printf("\n");
   
  }     
  else {
//                              
//                              Print out rank/thrd value
    if(multi_node){                  printf(" %20.20s  ", name);          wrap_cont += 23;} // Multi-node
    if(nranks == 1 &&  nthrds == 1 ){ printf(" "          );              wrap_cont += 1; }
    if(nranks != 1 &&  nthrds == 1 ){ printf("%4.4d ",rank);              wrap_cont += 5; }
    if(nranks == 1 &&  nthrds != 1 ){ printf("%4.4d ",thrd);              wrap_cont += 5; }
    if(nranks != 1 &&  nthrds != 1 ){ printf("%4.4d %4.4d ", rank, thrd); wrap_cont += 10;}

//                      MASK (just print first digit, header group indicates 10's digit)
//                      Print out mask (if 0, print - , == not set)
//                      Mask index == head group + least significant digit (sdigit)

//printf("else TpC=%d ncpus=%d proc_order=%d \n",info.TpC,ncpus,info.proc_order);
//printf("else SocSeq_Core1st=%d\n",SocSeq_Core1st);

if(info.proc_order == SocEO_Core1st)
{

//  @@ NOTE @@        // When forcing kernel view tpc is made equal to 1
//                    // Hence cores_per_socket will become HWTs_per_socket

 cores_per_socket=ncpus/(sockets*tpc);

 if(color == 1)
 {
                     // if kernel view, cores is set to ncpus
    no_occ = ( tpc > 1 )? '=' : '-';
    if( l == 's') no_occ = '-';

    k = 0;
    for(i=0;i<ncpus;i++){ 
      ii = i%cores;
      is = ii%2;

      if(ii == 0 && i != 0)
      {  
        memset(&str[k],' ',(size_t)wrap_cont);  k+=wrap_cont; // print wrap_continue spaces 
      }

      strncpy(&str[k],C[is],strlen(C[is])); k+=strlen(C[is]);

      lsdigit=ii;
      if(ii>9){lsdigit= ii - (ii/10)*10; }
      if(icpus[i] == 1) { str[k++] = lsdigit+'0'; }
      else              { str[k++] = no_occ ;     } 

      if(ii == cores-1)
      {
         strncpy(&str[k],RESET,strlen(RESET));  k+=strlen(RESET);    // Reset default color before adding return
         str[k++] = '\n';
      }

   } // for i ncpus
   str[k++] = '\0';                           // Add return and end of string
   printf("%s",str);

 } 
}

if( info.proc_order == SocSeq_Core1st)
{
  // sockets=2; // for checking (e.g. pretend multiple sockets on KNL)

  cores_per_socket=ncpus/(sockets*tpc);

 if(color == 1)
 {

  for(it=0;it<tpc;it++)
  {
    k=0;
                     // if kernel view, cores is set to ncpus
    no_occ = ( tpc > 1 && it == 0 )? '=' : '-';
    if( l == 's') no_occ = '-';

    if( tpc > 1 &&  it != 0 )
    {
      memset(&str[k],' ',(size_t)wrap_cont);  k+=wrap_cont; // print wrap_continue spaces
    }

    for(is=0;is<sockets;is++){ 
                                  // Add color for sockets
      if(sockets>1){ strncpy(&str[k],C[is],strlen(C[is])); k+=strlen(C[is]); }

      for(ic_socket=0;ic_socket<cores_per_socket;ic_socket++)
        {
          ic =                                is*cores_per_socket + ic_socket;
        //id =  it*sockets*cores_per_socket + is*cores_per_socket + ic;
          id =  it*sockets*cores_per_socket + is*cores_per_socket + ic_socket;

          lsdigit=ic;
          if(ic>9){lsdigit= ic - (ic/10)*10; }
          if(icpus[id] == 1){ str[k++] = lsdigit+'0'; }
          else              { str[k++] = no_occ;      }

        } // ic cores of a socket
    } // is sockets

                                // Reset default color before adding return
    if(sockets>1) {strncpy(&str[k],RESET,strlen(RESET));  k+=strlen(RESET);}

    str[k++] = '\n'; str[k++] = '\0';                           // Add return and end of string
    printf("%s",str);
    fflush(stdout);

  } // it hardware threads

 } // if color

}


if(info.proc_order == SocSeq_HWT1st)
{

 int color=1;

 if(color ==  1){

//  @@ NOTE @@         // When forcing kernel view tpc is made equal to 1
//                     // Hence cores_per_socket will become HWTs_per_socket
    cores_per_socket=ncpus/(sockets*tpc);

    for(it=0;it<tpc;it++){
      k=0;

    //no_occ = ( tpc > 1 && ! force_long && it == 0 )? '=' : '-';
      no_occ = ( tpc > 1 && it == 0 )? '=' : '-';
      if( l == 's') no_occ = '-';
 
      if( tpc > 1 &&  it != 0 )
      {
        memset(&str[k],' ',(size_t)wrap_cont);  k+=wrap_cont; // print wrap_continue spaces
      }
 
      for(is=0;is<sockets;is++){
        strncpy(&str[k],C[is],strlen(C[is])); k+=strlen(C[is]);
 
        for(ic_socket=0;ic_socket<cores_per_socket;ic_socket++){
          ic =  is*cores_per_socket + ic_socket;
          id =  ic*tpc + it;
 
          lsdigit=ic;
          if(ic>9){lsdigit= ic - (ic/10)*10; }
          if(icpus[id] == 1){ str[k++] = lsdigit+'0'; }
          else              { str[k++] = no_occ;      }
 
        } // ic cores of a socket
      } // is sockets
 
      strncpy(&str[k],RESET,strlen(RESET));  k+=strlen(RESET);    // Reset default color before adding return
      str[k++] = '\n'; str[k++] = '\0';                           // Add return and end of string
      printf("%s",str);
      fflush(stdout);

    } // it hardware threads


 }
 else
 {
    for(it=0;it<tpc;it++){
      no_occ = ( tpc > 1 && ! force_long && it == 0 )? '=' : '-';
      if( l == 's') no_occ = '-';
      for(ic=0;ic<cores;ic++){
         id = ic*tpc + it;

         if( tpc > 1 && ! force_long ){    // Doing cores

           if(ic == 0 && it != 0){
              printf("\n%*c",wrap_cont,' ');  // print wrap_continue spaces
              no_occ='-';
           }

           lsdigit=ic;
           if(ic>9){lsdigit= ic - (ic/10)*10; }
           if(icpus[id] == 1) printf("%1.1d",lsdigit);
           else               printf("%c"   ,no_occ );
           if(id == ncpus-1)  printf("\n");

         }
         else{
           lsdigit=ic;
           if(ic>9){lsdigit= ic - (ic/10)*10; }
           if(icpus[id] == 1) printf("%1.1d",lsdigit);
           else               printf("%c"   ,no_occ );
           if(id == ncpus-1)  printf("\n");
         }
      } // ic cores
    } // it hardware threads

    fflush(stdout);
 }
}


  } // end hdprt or else

}


/*
   program
       use iso_c_binding, only: C_CHAR, C_NULL_CHAR
       ...
       interface
         subroutine print_mask(          hd_prnt, name, multi_node, rank,thrd, ncpus,nranks,nthrds, icpus) bind(C,name="print_mask")
         use iso_c_binding, only: c_char
           integer,value, intent(in)  :: hd_prnt,       multi_node, rank,thrd, ncpus,nranks,nthrds
           character(kind=c_char)     ::          name(*)
           integer                    ::                                                            icpus(*)
         end subroutine
       end interface

       call print_mask(hd_prnt ,name//C_NULL_CHAR, name, multi_node, rank,thrd, ncpus,nranks,nthrds, icpus)
   end program
*/

int get_threads_per_node(){   
   int  TpC;
   int ret_lscpu=0;

   enum lscpu_method{popen,tmpfile}; 
   enum lscpu_method method;
   method = popen;

/*
    Returns the number of threads per core from lscpu Unix Utility command.
    Assumed that systems provide this command, and return a line with:
    'Thread(s) per core: #', where # is the number of hardware threads/core.
    Assume 1 thread per core if command is not available or if a non-number
    is returned.

    Should probably rewrite to extract and manipulate from /proc/cpuinfo
    to determine threads per core directly... later.
                                                    1/25/2017 Kent Milfeld
   
                   nthreads = CPU_COUNT_S(setsize, thread_siblings);
                   if (!nthreads) nthreads = 1; // in lscpu code
*/


   method = popen;

//                              // popen within parallel is broken
//                              // for intel 18 compiled code
#if defined(__INTEL_COMPILER)
#if __INTEL_COMPILER == 1800 && __INTEL_COMPILER_UPDATE >= 2
   method=tmpfile;
#endif
                               // Uh, They "fixed" it
                               // I should be able to call
                               // from a master/single region
                               // without getting this runtime warning:
//OMP: Warning #190: Forking a process while a parallel region is active is potentially unsafe.
#if __INTEL_COMPILER == 1900
   method=tmpfile;
#endif
#endif





#if defined(TpC_BY_POPEN)
   method=popen;
#endif

#if defined(TpC_BY_TMPFILE)
   method=tmpfile;
#endif

                                       // popen is broken on KNL, use tmpfile method
   if ( system("grep 'Phi(TM)' /proc/cpuinfo  > /dev/null 2>&1") == 0 ) method=tmpfile;


              //Make sure lscpu is available.
   ret_lscpu = system("lscpu > /dev/null 2>&1"); 

              //Report  Threads per Core.
   TpC=-1;
   if (ret_lscpu == 0) {
      if(method==popen  ){
         TpC = TpC_from_lscpu_popen();
      }
      if(method==tmpfile ){
         TpC = TpC_from_lscpu_tmpfile();
      }
   }

   if(TpC==-1)
   {
     //printf(" WARNING: could not execute lscpu to obtain threads per core (TpC). Using 1.\n");
     return 1; 
   }
   else { return TpC; }
   
}

int TpC_from_lscpu_tmpfile(void)
{
   // Temporary File and lscpu command variables:
   char file_template[22]="/tmp/AMASKtemp-XXXXXX"; //directory + template file name
   char cmd[ 128]        ="lscpu >";  //size to fit strlen(cmd)+strlen(file)+1
   char tmp_file[32];                 //size to fit strlen(file_template)+1
                                      // tmp_file hold lscpu report
   int status;

   FILE  *file;
   char    lscpu_line[4096];
   char    *cptr,*cptr_end;
   long  ldigit;
   int   TpC=-1;
   int i=0,j=0, ierr;

   // Create a temporary file (tmp_file)  with the output from lscpu:
   // tmp_file = cmd + file_template(after random value added for XXXXXX)
   // 
   memset(tmp_file,0,sizeof(tmp_file));
   strncpy(      tmp_file,file_template,strlen(file_template));


   status = mkstemp(tmp_file);
   if(status==-1)
   {
      printf("WARNING: mkstemp failed. OK, amask will default to kernel view.\n");
      unlink(tmp_file);
   }

   if(status!=-1){
      memcpy(cmd+strlen(cmd),tmp_file,strlen(tmp_file)+1);
      status=system(cmd);
      if(status==-1) printf("WARNING: lscpu failed. OK, amask will default to kernel view.\n");
   }


   if(status!=-1){
      file = fopen(tmp_file, "r");
      if (file == NULL)
      {
          ierr=errno;
          printf("WARNING: fopen failed. OK, amask will default to kernel view.\n");
          fprintf(stderr,"        code file: %s     line: %d\n", __FILE__,__LINE__);
          printf("        file: %s  errno-string: %s\n",tmp_file,strerror(ierr));
          return 1;
      }
   
      while (EOF != fscanf(file, "%[^\n]\n", lscpu_line))
      {
       j++;
          if( strstr(lscpu_line,"Thread(s) per core:") != NULL)
          { 
            cptr=lscpu_line;
            cptr_end=lscpu_line;
            while (*(cptr_end++)!='\0')
            while( *cptr){
              i++;
              if(isdigit(*cptr)){
                //printf(" DIGIT %c -- %s     ->%d %d\n",*cptr, lscpu_line,i,j); 
                  ldigit = strtol(cptr, &cptr_end, 10); //printf("ldigit=%ld\n",ldigit);
                  TpC= (int) ldigit;
                  break;
              } 
            cptr++;
            }
          }
      }
      fclose(file);

      // Remove temporary file (tmp_file)
      unlink(tmp_file);

   }

    return TpC;
}

int TpC_from_lscpu_popen(void)
{
   FILE* fp;
   char c_tpc[5], *endptr;
   long l_tpc;
   int  TpC, ierr;
                // Pickup # threads/core
      fp = popen("lscpu | grep -e 'Thread(s) per core:' | awk '{print $NF}'", "r");
      fgets(c_tpc, 5, fp);
      l_tpc= strtol(c_tpc, &endptr, 10);
      ierr=errno;
      if ((ierr == ERANGE && (l_tpc == LONG_MAX || l_tpc == LONG_MIN))
           || (ierr != 0 && l_tpc == 0) || endptr == c_tpc ) {
         printf("WARNING: popen method failed. OK, amask will default to kernel view.\n");
         fprintf(stderr,"        code file: %s     line: %d\n", __FILE__,__LINE__);
         printf("        errno-string: %s\n",strerror(ierr));
         TpC=-1;
      }
      else
      {  TpC= (int)l_tpc;  }

      return TpC;
}

void get_node_info(nodeinfo &info)
{

   // Temporary File and lscpu command variables:
   char file_template[22]="/tmp/AMASKtemp-XXXXXX"; //directory + template file name
   char cmd[ 128]        ="lscpu   >";  //size to fit strlen(cmd)+strlen(file)+1
   char tmp_file[34];                 //size to fit strlen(file_template)+1
                                      // tmp_file hold lscpu report
   int status;
 
   FILE *file;
   char lscpu_line[4096];
   long ldigit;
   long int core_num[2], socket_id[2], core_id[2];
 
   int  ierr, knt=0;
   char  *cptr, *ptr_start, *ptr_end;
   int   c_diff, s_diff;

   FILE *fd_sibling_file;
   char name_sibling_file[]="/sys/devices/system/cpu/cpu0/topology/thread_siblings_list";
   char fscanf_line[4096];


   // Create a temporary file (tmp_file)  with the output from lscpu:
   // tmp_file = cmd + file_template(after random value added for XXXXXX)
   // 
   memset(tmp_file,0,sizeof(tmp_file));                        //zero out tmp_file
   strncpy(      tmp_file,file_template,strlen(file_template));//put in file name

   status = mkstemp(tmp_file); //make sure tmp_file can be made.
   if(status==-1) {
      printf("WARNING: mkstemp failed. OK, amask will default to kernel view.\n");
      unlink(tmp_file);
      info.status=0;
      //return info;
      return ;
   }

   memcpy(cmd+strlen(cmd),tmp_file,strlen(tmp_file)+1);
   status=system(cmd);
   if(status!=0) {
      printf("WARNING: lscpu failed. OK, amask will default to kernel view.\n");
      unlink(tmp_file);
      info.status=0;
      //return info;
      return ;
   }

   file = fopen(tmp_file, "r");
   if (file == NULL)
   {
      ierr=errno;
      printf("WARNING: fopen failed. OK, amask will default to kernel view.\n");
      fprintf(stderr,"        code file: %s     line: %d\n", __FILE__,__LINE__);
      printf("        file: %s  errno-string: %s\n",tmp_file,strerror(ierr));
      unlink(tmp_file);
      info.status=0;
      //return info;
      return ;
   }

   while (EOF != fscanf(file, "%[^\n]\n", lscpu_line))
   {
      if( strstr(lscpu_line,"Thread(s) per core:") != NULL)
      { 
        cptr    =lscpu_line;
        while( *cptr){         //now let's find the first digit, and extract #
          if(isdigit(*cptr)){
             info.TpC = (int) strtol(cptr, NULL, 10); //printf("ldigit=%ld\n",ldigit);
             break;
          }
          cptr++; 
        }
      }

      if( strstr(lscpu_line,"Socket(s):") != NULL)
      { 
        cptr    =lscpu_line;
        while( *cptr){         //now let's find the first digit, and extract #
          if(isdigit(*cptr)){
             info.nsockets = (int) strtol(cptr, NULL, 10); //printf("ldigit=%ld\n",ldigit);
             break;
          }
          cptr++; 
        }
      }

      if( strstr(lscpu_line,"Core(s) per socket:") != NULL)
      {
        cptr    =lscpu_line;
        while( *cptr){         //now let's find the first digit, and extract #
          if(isdigit(*cptr)){
             info.CpS = (int) strtol(cptr, NULL, 10); //printf("ldigit=%ld\n",ldigit);
             break;
          }
          cptr++; 
        }
      }

                                                  //NUMA node0 CPU(s): may also exist.
                                                  //&& makes sure CPU is at begging.
      if( strstr(lscpu_line,"CPU(s):") != NULL &&  strstr(lscpu_line,"CPU(s):") == lscpu_line)
      {
        cptr    =lscpu_line;
        while( *cptr){         //now let's find the first digit, and extract #
          if(isdigit(*cptr)){
             info.nproc_ids = (int) strtol(cptr, NULL, 10); //printf("ldigit=%ld\n",ldigit);
             break;
          }
          cptr++; 
        }
      }
   }

   if( info.nproc_ids != info.nsockets*info.CpS*info.TpC ){
      unlink(tmp_file);
     info.status=0;
      //return info;
      return ;
   }

   info.ncores = info.nsockets*info.CpS;
   info.proc_order = Order_Error;

      // if TpC >1, should verify in file=/sys/devices/system/cpu/cpu0/topology/thread_siblings_list
      // Cat file, should not be only "0", must have "#,#", or "#=#".
      // On small systems TpC has been known not to report TpC correctly (1 when should be > 1).


   strncpy(&cmd[6],"-p",2);        // new command, -p give csv for each proc_id,core_id,socket_id ...

   status=system(cmd);
   if(status!=0) {
      printf("WARNING: lscpu -p failed. OK, amask will default to kernel view.\n");
      unlink(tmp_file);
      info.status=0;
      fclose(file);
      unlink(tmp_file);            // Remove temporary file (tmp_file)
      //return info;
      return ;
   }

   file = fopen(tmp_file, "r");

   while (EOF != fscanf(file, "%[^\n]\n", lscpu_line))
   {
     if( strstr(lscpu_line,"#") == NULL)
     {
       ptr_start = lscpu_line;
       ptr_end   = lscpu_line;
       while ( isdigit(*ptr_end++) ){;} ptr_end--;   //{;} keeps xl compiler happy
 
       core_num[knt] = strtol(ptr_start, NULL, 10);   // 1st value
 
       ptr_end++;
       ptr_start=ptr_end;
       while ( isdigit(*ptr_end++) ){;} ptr_end--;
 
       core_id[knt] = strtol(ptr_start, NULL, 10);  // 2nd value
 
       ptr_end++;
       ptr_start=ptr_end;
       while ( isdigit(*ptr_end++) ){;} ptr_end--;
 
       socket_id[knt] = strtol(ptr_start, NULL, 10);    // 3rd value
 
       knt++;
     }
     if(knt == 2) break;
   }

   info.status=1;
 
   s_diff= (int)( socket_id[1]-socket_id[0] );
   c_diff= (int)(   core_id[1]-  core_id[0] );
printf("s_diff=%d c_diff=%d\n",s_diff, c_diff);
printf("socket_id[0]=%d,socket_id[1]=%d,  core_id[0]=%d, core_id[1]=%d \n",socket_id[0],socket_id[1], core_id[0],core_id[1] ); 

   if( s_diff == 0 && c_diff == 0 ) info.proc_order=SocSeq_HWT1st;
   if( s_diff  > 0 && c_diff  > 0 ) info.proc_order=SocEO_Core1st;
   if( s_diff == 0 && c_diff  > 0 ) info.proc_order=SocSeq_Core1st;
   if( s_diff  > 0 && c_diff == 0 ) info.proc_order=Order_Error;

   if( info.TpC == 1  ){    // lscpu may be wrong about TpC=1; verify
      fd_sibling_file = fopen(name_sibling_file, "r");
      if (fd_sibling_file == NULL) {
        info.TpC_verified=0;
      }
      else{
        fscanf(fd_sibling_file, "%[^\n]\n", fscanf_line);
        if ( fscanf_line[1] == '\0') info.TpC_verified=1;
      //if ( fscanf_line[1] == '\0' && info.TpC == 1 ) info.TpC_verified=1; // OK
      //if ( fscanf_line[1] == '\0' && info.TpC  > 1 ) info.TpC_verified=0; //probably will never happen
      //if ( fscanf_line[1] != '\0' && info.TpC  > 1 ) info.TpC_verified=1; // OK
      //if ( fscanf_line[1] != '\0' && info.TpC == 1 ) info.TpC_verified=0; //probable wrong case
      }
   }
   info.TpC_verified=1;

   fclose(file);
   unlink(tmp_file);            // Remove temporary file (tmp_file)
   //return info;
   return ;
}
