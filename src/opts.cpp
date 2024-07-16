#include "opts.h"

// static var defaults must be set outside class (until C++11)
int  Maskopts::w=0;
char Maskopts::v='s', Maskopts::p='f', Maskopts::h='-';
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
                                     {"help",       no_argument,       0,  'h' },
                                     {0,            0,                 0,   0  }
                                   };

   argcnt = 0;
   v='0';//w='0';p='f';h='-';   // use v='s' to make single row the default.
   //                    nvc++ rightfully wants getopt_long to return int. (But we need char.)
   while ((cint = getopt_long(argc, argv, "w:v:p:h", long_opts, &long_index)) != -1) {
      c=(char)cint; 
      switch (c) {
printf(" switch, c=%c\n",c); 
        case 'w':
                w=atoi(optarg);
                argcnt++;  
                break;  
        case 'v':
                c_arg=optarg[0];
                if(c_arg != 's' && c_arg != 'm'){ print_usage_cmdln('v'); }
                if(c_arg == 's' || c_arg == 'm'){ v=c_arg; }
                argcnt++;  
                break;  
        case 'p':
                c_arg=optarg[0];
                if(c_arg != 'f' && c_arg != 's'){ print_usage_cmdln('l'); }
                if(c_arg == 'f' || c_arg == 's'){ p=c_arg; }
                argcnt++;  
                break;  
        case 'h':
                h='h';
                argcnt++;
                print_usage_cmdln('h'); exit(0);
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

string env_vars[] = { "AMASK_WAITSECS", "AMASK_LISTBY", "AMASK_PRINTSPEED"};

   argcnt=0;

   env_p = std::getenv( env_vars[0].c_str() );
   if ( env_p != 0 )            { w = atoi(env_p); argcnt++; }

   env_p = std::getenv( env_vars[1].c_str() );
   if ( env_p != 0 )            { c = env_p[0];    argcnt++;

      if( c != 's' && c != 'm') { print_usage_env("AMASK_VIEW");  }
      if( c == 's' || c == 'm') { v=c; }

   }

   env_p = std::getenv( env_vars[2].c_str() );
   if ( env_p != 0 )            { c = env_p[0];    argcnt++;

      if( c != 'f' && c != 's') { print_usage_env("AMASK_PRINTSPEED");  }
      if( c == 'f' || c == 's') { p=c; }
   }

   return argcnt;
}

void Maskopts::print_usage_cmdln(char c_err) {
   printf("Usage: amask_mpi|omp|hybrid [-w|-waitsecs #] [-v|-view s|m] [-p|-printspeed f|s] [-h|-help]  \n"); 
   printf("                    Wait loads processes for # sec. View uses single/multiple lines for mask.\n"); 
   printf("                    Printspeed fast (immediate) or slow 0.3sec delay. Help prints this usage.\n"); 
   if(c_err!='h') exit(1); // printf("       Command line option problem (found %c for option value).\n", c_err ); 
} 

void Maskopts::print_usage_env(const char *c_err) {
   printf("Usage: AMASK_WAITSECS=<int>(sec) AMASK_VIEW=s|m (single/multi line) AMASK_PRINTSPEED= f|s\n"); 
   if(c_err != NULL) printf("       Problem with %s option.\n", c_err ); 
} 
