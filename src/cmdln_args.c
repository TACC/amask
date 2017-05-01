/* Routine parses command line for nsec and value
   returns integer value in nsec variable

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

/*   DRIVER for checking and extending

void cmdln_get_nsec_or_help(int * nsec, int argc, char * argv[]);
int main(int argc, char ** argv){
int nsec=10;

   cmdln_get_nsec_or_help( &nsec, argc, argv);
   printf("nsec=%d\n", nsec);
}
*/

void cmdln_get_nsec_or_help(int * nsec, int argc, char * argv[]){

// Read one command line argument
//    if only digits, argument is value for nsec
//    if some form of help request, print Usage

int i,j;
                          // Line parsing
char digits[10] = { "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" };
int  knt = 0;



   if ( argc == 2 ) // Only 1 option allowed   help or number_of_seconds
   {
                   //  Is this an int (nothing but digits)? If so store in nsec. (1.)
                   //  nsec has a default value of 10.
       for(j=0;j<strlen(argv[1]);j++) for(i=0;i<10;i++)
          if(argv[1][j] == digits[i] ){knt++;};

       if(knt == strlen(argv[1])){ *nsec = atoi(argv[1]); }  //if all are ints

                   // Is this a help request?
       else if (! strcmp(argv[1],   "help") ||
                ! strcmp(argv[1],  "-help") ||
                ! strcmp(argv[1], "--help") )
                printf("Usage: %s [ string [-[-]]help | int seconds] \n", argv[0]);

       else { printf(" Did not understand argument \" %s \".\n",argv[1]); exit(1); }
   }
   if ( argc > 2 ) printf("Usage: %s [ string [-[-]]help | int seconds] \n", argv[0]);
}

