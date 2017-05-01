/* Routine sets affinity to a single cpu_id (procid)
                                           Kent Milfeld
                                           206/07/13

*/

#include <sched.h>
#include <unistd.h>
#include <cstdlib>

int map_to_procid( int procid){
    cpu_set_t        mask;
    CPU_ZERO(       &mask);
    CPU_SET(procid,  &mask);
    return( sched_setaffinity( (pid_t) 0 , sizeof(mask), &mask ) );
}

void  map_to_procid_(int *procid){
int i;
    i=map_to_procid(     *procid);
    if( i!=0 ) exit(1);
}
