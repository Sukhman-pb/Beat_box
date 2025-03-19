#ifndef _BTN_STATEMACHINE_H_
#define _BTN_STATEMACHINE_H_

#include <stdbool.h>

void BtnStateMachine_rotaryInit(void);
void BtnStateMachine_joyInit(void);
void BtnStateMachine_RotBtnCleanup(void);
void BtnStateMachine_JoyCleanup(void);

int BtnStateMachine_getValue(void);
bool BtnStateMachine_isPressed(void);
int BtnStateMachine_counterPressed(void);

// TODO: This should be on a background thread (internal?)
void BtnStateMachine_rotary(void);
void BtnStateMachine_joystick(void);

#endif