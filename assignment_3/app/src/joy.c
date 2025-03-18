#include "hal/joy_stick_control.h"
#include <stdio.h>

int main(){
    int i2c_file_desc = joystick_init();
    Joystick_direction y = joystick_get_y(i2c_file_desc);
    while(y != LEFT){
        y = joystick_get_y(i2c_file_desc);
        printf("Joystick y direction: %d\n", y);
    }
    printf("Joystick y direction: %d\n", y);
    joystick_close(i2c_file_desc);
    return 0;
}