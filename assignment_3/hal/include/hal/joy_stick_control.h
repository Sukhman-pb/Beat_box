#ifndef JOY_STICK_CONTROL_H
#define JOY_STICK_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "btn_statemachine.h"
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

int joystick_getVol(void);

void joystick_setVol(int vol);

int joystick_getScreenCount(void);

void joystick_button(void);

void joystick_cancel(void);
//closes the joystick
//parameter description
//i2c_file_desc: the file descriptor of the i2c bus that we get from the joystick_init function
void joystick_close(int i2c_file_desc);
#endif