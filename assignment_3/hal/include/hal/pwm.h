//this file provides a hal for PWM module by seperating out changing frequencies of the emitter
#ifndef PWM_H
#define PWM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//path of the file that we need to change
#define PWM_PATH "/dev/hat/pwm/GPIO12/"

//init function to initialize the values
void init_pwm(void);

//function to change the duty_cycles
void pwm_set_duty_cycle(int duty_cycle);

//function to change the period
void pwm_set_period(int period);
//function to enable and disable the emitter
void pwm_enable(int enable);
//function to change the frequency with the provided value
void change_frequency(int frequency);
//closing fucniton
void close_pwm(void);
#endif
