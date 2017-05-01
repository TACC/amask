#include <iostream>
#include <stdio.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <cstdlib>
#include <cstring>

using namespace std;

class Maskopts{

  private:
  static int  w;  // wait time (sec)
  static char l,p,u;  // listing, printing and usage
  static bool setbycmdline ;

  public:
    int argcnt;
    int argc;
    char **argv;

    Maskopts();
    Maskopts(int argc, char** argv);

    void set_w(int  sec ){ this->w = sec;  }
    void set_l(char cors){ this->l = cors; }
    void set_p(char fors){ this->p = fors; }
    void set_u(char uor0){ this->u = uor0; }

    int  get_w(){ return this->w; }
    char get_l(){ return this->l; }
    char get_p(){ return this->p; }
    char get_u(){ return this->u; }

    bool is_set_by_cmdline(){return this->setbycmdline;}

    void print_usage_cmdln(char  c_err) ;
    void print_usage_env(  char *c_err) ;

    int my_cmdln_opts();
    int my_env_opts();

};
