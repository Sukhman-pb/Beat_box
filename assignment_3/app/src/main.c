#include "hal/joy_stick_control.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include "hal/helper.h"
#include "hal/rotar_statemachine.h"
#include "hal/gpio.h"
#include "hal/knob.h"
#include "draw_stuff.h"
#include "hal/accel.h"

static int screen = 0;
static int beat_num = 0, bpm = 0;
static int count = 0;
static bool x=0,y=0,z=0;
int main(){
    DrawStuff_init();
    Gpio_initialize();
    accel_init();
    int i2c_file_desc = joystick_init();
    knob_init();
    int shutdown_ = 0;
  
    while(1){
        count = joystick_getVol();
        printf("volume is :%d\n", count);
        screen = joystick_getScreenCount();
        printf("Screen number is :%d\n", screen);
        beat_num = knob_getBeatNum();
        printf("The beat to play is :%d\n", beat_num);
        bpm = knob_getBPM();
        printf("The beat per minute is:%d\n", bpm);
        x = get_accel(1);
        printf("x is %d\n", x);
        y = get_accel(2);
        printf("y is %d\n", y);
        z = get_accel(3);
        printf("z is %d\n", z);
        sleep_ms(500);
        shutdown_ ++;
        if(shutdown_ == 50){
            joystick_cancel();
            knob_cancel();
            break;}
    }
    knob_close();
    accel_close();
    joystick_close(i2c_file_desc);
    Gpio_cleanup();
    DrawStuff_cleanup();
    return 0;
}