#ifndef HELPER_H
#define HELPER_H

//from assignment description
//returns the current time in milliseconds
//parametric description: void = no parameters
//return description: long long = the current time in milliseconds
long long get_time_in_ms(void);

//from assignment descriptions
//sleeps for a given amount of milliseconds
//parametric description: long long ms = the amount of milliseconds to sleep
//return description: void = no return
void sleep_ms(long long ms);
#endif