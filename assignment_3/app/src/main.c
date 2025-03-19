#include "hal/joy_stick_control.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include "hal/helper.h"
#include "hal/rotar_statemachine.h"
#include "hal/gpio.h"
#include "hal/knob.h"
#include "draw_stuff.h"

static int screen = 0;
static bool running = true;
static int beat_num = 0, bpm = 0;
static int count = 0;
//static pthread_mutex_t data = PTHREAD_MUTEX_INITIALIZER;
static pthread_t lcd;
void* lcd_update_thread(void* arg){
    (void)arg;
    char lcdBuff[128];
    while(running){
        //pthread_mutex_lock(&data);
        printf("1\n");
        snprintf(lcdBuff, sizeof(lcdBuff),
        "\033[1mBeat %d\033\n"
        "volume %d   bpm %d\n "
                 ,beat_num, count,bpm);
        //pthread_mutex_unlock(&data);
        printf("2\n");
        DrawStuff_updateScreen(lcdBuff);
        sleep_ms(1000);
    }
    return NULL;
}
int main(){
    DrawStuff_init();
    Gpio_initialize();
    int i2c_file_desc = joystick_init();
    knob_init();
    pthread_create(&lcd, NULL, lcd_update_thread, NULL);
    int shutdown_ = 0;
  
    while(1){
        count = joystick_getVol();
        printf("volume is :%d\n", count);
        screen = joystick_getScreenCount();
        printf("Screen number is :%d\n", screen%3);
        beat_num = knob_getBeatNum();
        printf("The beat to play is :%d\n", beat_num%3);
        bpm = knob_getBPM();
        printf("The beat per minute is:%d\n", bpm);
        sleep_ms(50);
        shutdown_ ++;
        if(shutdown_ == 200){
            joystick_cancel();
            knob_cancel();
            running = false;
            break;}
    }
    knob_close();
    joystick_close(i2c_file_desc);
    Gpio_cleanup();
    DrawStuff_cleanup();
    pthread_join(lcd, NULL);
    return 0;
}