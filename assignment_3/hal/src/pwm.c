#include "hal/pwm.h"


void init_pwm(void){
    pwm_set_duty_cycle(0);     // Reset duty cycle
    change_frequency(10);    // Set frequency to 10 Hz
    pwm_set_duty_cycle(5000000); // Set 50% duty cycle
    pwm_enable(1);
}

void pwm_write(const char *filename, int value) {
    char path[100];
    snprintf(path, sizeof(path), "%s%s", PWM_PATH, filename);
    
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Error opening PWM file");
        return;
    }
    
    fprintf(fp, "%d", value);
    fclose(fp);
}

void pwm_set_duty_cycle(int duty_cycle) {
    pwm_write("duty_cycle", duty_cycle);
}

void pwm_set_period(int period) {
    pwm_write("period", period);
}

void pwm_enable(int enable) {
    pwm_write("enable", enable);
}

void change_frequency(int frequency){
    int period_ns = 0;
    if(frequency > 2){      //adding condition because of hardware limits
        period_ns = 1000000000 / frequency;
    }
    pwm_set_period(period_ns);
    //pwm_set_duty_cycle(period_ns / 2);

    if(frequency > 2){      //adding condition because of hardware limits
        pwm_set_duty_cycle(period_ns / 2);
    }
    else {
     pwm_set_duty_cycle(5000000);   
    }
}
void close_pwm(void){
    pwm_enable(0);
}
