#include "opts.h"

// static var defaults must be set outside class (until C++11)
int  Maskopts::w=0;
char Maskopts::v='c', Maskopts::p='s', Maskopts::u='-';
bool Maskopts::setbycmdline=false;


// ---------------------------------------------------------------------------------
// COMMAND LINE VARIABLE PATH

Maskopts::Maskopts(void){
   argcnt=my_env_opts();
}
Maskopts::Maskopts(int myargc, char** myargv){
   argc = myargc;
   argv = myargv;
   setbycmdline = true;

   argcnt=my_cmdln_opts();
}

int Maskopts::my_cmdln_opts(){

//char  long_opt argument:
// w wait-time <int>; v view kernel|core;  p print-speed fast|slow; u usage no_arg
// http://stackoverflow.com/questions/7489093/getopt-long-proper-way-to-use-it

char c, c_arg; int cint, long_index=0;

// Options
static struct option long_opts[] = {
                                     {"waitsecs",   required_argument, 0,  'w' },
                                     {"view",       required_argument, 0,  'v' },
                                     {"printspeed", required_argument, 0,  'p' },
                                     {"usage",      no_argument,       0,  'u' },
                                     {0,            0,                 0,   0  }
                                   };

   argcnt = 0;
   v='0';//w='0';p='0';u='0';
   //                    nvc++ rightfully wants getopt_long to return int. (But we need char.)
   while ((cint = getopt_long(argc, argv, "w:v:p:u", long_opts, &long_index)) != -1) {
      c=(char)cint; 
      switch (c) {
 
        case 'w':
                w=atoi(optarg);
                argcnt++;  
                break;  
        case 'v':
                c_arg=optarg[0];
                if(c_arg != 'c' && c_arg != 'k'){ print_usage_cmdln('v'); }
                if(c_arg == 'c' || c_arg == 'k'){ v=c_arg; }
                argcnt++;  
                break;  
        case 'p':
                c_arg=optarg[0];
                if(c_arg != 'f' && c_arg != 's'){ print_usage_cmdln('l'); }
                if(c_arg == 'f' || c_arg == 's'){ p=c_arg; }
                argcnt++;  
                break;  
        case 'u':
                u='u';
                argcnt++;
                print_usage_cmdln('u'); exit(0);
        case '?':
                print_usage_cmdln('?');

      }// end switch

   } //end while

   return argcnt;

} //end my_cmdln_opts


// ---------------------------------------------------------------------------------
// ENVIRONMENT VARIABLE PATH
int Maskopts::my_env_opts(){

char c;
const char * env_p;

string env_vars[] = { "CMASK_WAITSECS",   "CMASK_LISTBY",
                      "CMASK_PRINTSPEED", "CMASK_USAGE"      };

   argcnt=0;

   env_p = std::getenv( env_vars[0].c_str() );
   if ( env_p != 0 )            { w = atoi(env_p); argcnt++; }

   env_p = std::getenv( env_vars[1].c_str() );
   if ( env_p != 0 )            { c = env_p[0];    argcnt++;

      if( c != 'c' && c != 'k') { print_usage_env("CMASK_VIEW");  }
      if( c == 'c' || c == 'k') { v=c; }

   }

   env_p = std::getenv( env_vars[2].c_str() );
   if ( env_p != 0 )            { c = env_p[0];    argcnt++;

      if( c != 'f' && c != 's') { print_usage_env("CMASK_PRINTSPEED");  }
      if( c == 'f' || c == 's') { p=c; }
   }

   env_p = std::getenv( env_vars[3].c_str() );
   if ( env_p != 0 )           { u = 'u';          argcnt++;  
                                 print_usage_env(NULL); exit(0);  }

   return argcnt;
}

void Maskopts::print_usage_cmdln(char c_err) {
   printf("NOTICE: -ls and -lc options are now -vk and -vc, respectively. (Kernel and Core Views)\n"); 
   printf("Usage: amask_mpi|omp|hybrid|serial [-w|-waitsecs #] [-v|-view k|c] [-p|-printspeed f|s] [-u|-usage]\n"); 
   if(c_err!='u') printf("       Problem with %c option.\n", c_err ); 
} 

void Maskopts::print_usage_env(const char *c_err) {
   printf("Usage: CMASK_WAITSECS=<int>(sec) CMASK_VIEW=kernel|core CMASK_PRINTSPEED=fast|slow CMASK_USAGE=u \n"); 
   if(c_err != NULL) printf("       Problem with %s option.\n", c_err ); 
} 
