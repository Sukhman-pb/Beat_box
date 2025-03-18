#ifndef JOY_STICK_CONTROL_H
#define JOY_STICK_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
//for switcsh cases and modularity
typedef enum{
    UP,   //0
    DOWN, //1
    CENTER,//2
    LEFT, //3
} Joystick_direction;

//initializing the joystick
//parameter description
//void = no parameters required
//returns the int file descriptor of the i2c bus
int joystick_init(void);



//gets the y direction of the joystick
//parameter description
//i2c_file_desc: the file descriptor of the i2c bus that we get from the joystick_init function
//returns up or down or center
Joystick_direction joystick_get_y(int i2c_file_desc);


//closes the joystick
//parameter description
//i2c_file_desc: the file descriptor of the i2c bus that we get from the joystick_init function
void joystick_close(int i2c_file_desc);
#endif