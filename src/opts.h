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
  static char v,p,h;  // view, printing and usage
  static bool setbycmdline ;

  public:
    int argcnt;
    int argc;
    char **argv;

    Maskopts();
    Maskopts(int argc, char** argv);

    void set_w(int  sec ){ this->w = sec;  }
    void set_v(char cork){ this->v = cork; }
    void set_p(char fors){ this->p = fors; }
    void set_h(char hor0){ this->h = hor0; }

    int  get_w(){ return this->w; }
    char get_v(){ return this->v; }
    char get_p(){ return this->p; }
    char get_h(){ return this->h; }

    bool is_set_by_cmdline(){return this->setbycmdline;}

    void print_usage_cmdln(      char  c_err) ;
    void print_usage_env(  const char *c_err) ;

    int my_cmdln_opts();
    int my_env_opts();

};
