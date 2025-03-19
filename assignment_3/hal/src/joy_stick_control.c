#include "hal/joy_stick_control.h"
#include "hal/i2c_bus.h"
#include "hal/helper.h"
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <stdatomic.h>
#include <pthread.h>
#define Y_THRESHOLD 700  //hard coded threshold
static uint16_t centre ;
static bool isInit = false;
static int counter = 80;
int i2c_file_desc = 0;
int count= 0;
int screen = 0;
pthread_t direction, button;
//this is for ending the for loops inside the 
atomic_int fin= 0;

void* joy_vol(void* arg){
    (void)arg;
    while(1){
        joystick_get_y(i2c_file_desc);
        if(fin == 100){break;}
    }
    return NULL;
}

void* joy_screen(void* arg){
    (void)arg;
    while(1){
        joystick_button();
        if(fin == 100){break;}
    }
    return NULL;
}
int joystick_init(void){
    //initialize the i2c bus
    assert(!isInit);
    i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, JOYSTICK_ADDRESS);
    if(i2c_file_desc < 0){
        perror("Error initializing the i2c bus\n");
        return -1;
    }
   write_register(i2c_file_desc, REG_CONFIGURATION, CHANNEL_Y);;       
   sleep_ms(50);
   uint16_t raw = read_register(i2c_file_desc, REG_DATA);
   centre  = ((raw & 0xFF) << 8) | ((raw & 0xFF00) >> 8);
   centre  = centre >> 4;
   isInit = true;
   BtnStateMachine_joyInit();
   pthread_create(&direction, NULL, joy_vol, NULL);
   pthread_create(&button, NULL, joy_screen, NULL);
   return i2c_file_desc;
}


Joystick_direction joystick_get_y(int i2c_file_desc){
    assert(isInit);
    write_register(i2c_file_desc, REG_CONFIGURATION, CHANNEL_Y);       //Y axis
    sleep_ms(50);
    uint16_t raw = read_register(i2c_file_desc, REG_DATA);
    uint16_t value = ((raw & 0xFF) << 8) | ((raw & 0xFF00) >> 8);
    value  = value >> 4;
     if (value < centre - Y_THRESHOLD) {
        if(counter <= 95){ counter += 5;}
        return UP;
    } else if (value > centre + Y_THRESHOLD) {
        if(counter>=5){counter -= 5;}
        return DOWN;
    } else {
        return CENTER;
    }
}

int joystick_getVol(){
    assert(isInit);
    return counter;
}

void joystick_button(){
    assert(isInit);
    BtnStateMachine_joystick();
}

int joystick_getScreenCount(){
    assert(isInit);
    return BtnStateMachine_counterPressed()%3;
}

void joystick_cancel(){
    assert(isInit);
    pthread_cancel(direction);
    pthread_cancel(button);
}


void joystick_close(int i2c_file_desc){
    //closes the i2c bus
    assert(isInit);
    isInit = false;
    pthread_join(direction,NULL);
    pthread_join(button,NULL);
    close_i2c_bus(i2c_file_desc);
    BtnStateMachine_JoyCleanup();
}