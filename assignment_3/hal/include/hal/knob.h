#ifndef _KNOB_H
#define _KNOB_H

void knob_init(void);
void knob_cancel(void);
int knob_getBeatNum(void);
int knob_getBPM(void);
void knob_setBeatNum(int val);
void knob_setBPM(int val);
void knob_close(void);
#endif