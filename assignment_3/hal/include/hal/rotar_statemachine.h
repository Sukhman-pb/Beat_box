#ifndef  ROTAR_STATEMACHINE_H_
#define  ROTAR_STATEMACHINE_H_
#include <stdbool.h>
//This file uses the gpio.h in the background which is imported in .c file as we are not using the gpio anywhere else.
//Basic idea taken from this link downlodable file with file name btn_statemachine.h https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/rotary_encoder/


/*
*This function is used to initialize the state machine
*It sets the lines and initializes the state machine
*It also creates a thread to run the state machine
*reset all the counter to 0
*/
void rotar_state_machine_init(void);

/*This function is used to clean up the state machine
*It closes the lines and cancels the thread and then join it
*Closes the gpio lines
*/
void rotar_state_machine_cleanup(void);

/*This function is used to get the value of the counter set by the state machine
*It returns the value of the counter
*/
int rotar_state_machine_get_value(void);

/*This function is used to get the counter or set the counter value on every rotation
* This is event based function that runs the state machine and changes the counter value based on 4 states and how it is cycled through it
*/
void rotar_state_machine_do_state();

//This function is used to set the counter value to the value passed as argument
void set_counter(int value);
#endif