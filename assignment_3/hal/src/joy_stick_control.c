#include "hal/joy_stick_control.h"
#include "hal/i2c_bus.h"
#include "hal/helper.h"
#include <errno.h>
#include <assert.h>
#include <time.h>
#define Y_THRESHOLD 300  //hard coded threshold
static uint16_t centre ;

int joystick_init(void){
    //initialize the i2c bus
    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, JOYSTICK_ADDRESS);
    if(i2c_file_desc < 0){
        perror("Error initializing the i2c bus\n");
        return -1;
    }
    //gets the centre value of the x axis
    //we can also get the value from the y axis
   write_register(i2c_file_desc, REG_CONFIGURATION, CHANNEL_Y);;       
   sleep_ms(50);
   uint16_t raw = read_register(i2c_file_desc, REG_DATA);
   centre  = ((raw & 0xFF) << 8) | ((raw & 0xFF00) >> 8);
   centre  = centre >> 4;
    
   return i2c_file_desc;
}


Joystick_direction joystick_get_y(int i2c_file_desc){
    write_register(i2c_file_desc, REG_CONFIGURATION, CHANNEL_Y);       //Y axis
    sleep_ms(50);
    uint16_t raw = read_register(i2c_file_desc, REG_DATA);
    uint16_t value = ((raw & 0xFF) << 8) | ((raw & 0xFF00) >> 8);
    value  = value >> 4;
     if (value < centre - Y_THRESHOLD) {
        return UP;
    } else if (value > centre + Y_THRESHOLD) {
        return DOWN;
    } else {
        return CENTER;
    }
}
void check_y_pushed(int i2c_file_desc){
    Joystick_direction y = joystick_get_y(i2c_file_desc);
    bool printed = false;
    while(y != CENTER){
        if(printed == false){
        printf("Please let go off Joystick\n");
        printed = true;
        }
        y = joystick_get_y(i2c_file_desc);
    }        
}


void joystick_close(int i2c_file_desc){
    //closes the i2c bus   
    close_i2c_bus(i2c_file_desc);
}