#include "hal/helper.h"
#include <time.h>
//from assignment description
long long get_time_in_ms(void){
    struct timespec spec;
    //gets the current time and fills the spec struct
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoseconds = spec.tv_nsec;
    long long milliseconds = seconds * 1000 + nanoseconds / 1000000;    
    return milliseconds;
}

//from assignment description
void sleep_ms(long long ms){
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delay_ns = ms * NS_PER_MS;
    int seconds = delay_ns / NS_PER_SECOND;
    int nanoseconds = delay_ns % NS_PER_SECOND;

    struct timespec reqdelay = {seconds, nanoseconds};
    nanosleep(&reqdelay, (struct timespec *)NULL);
}