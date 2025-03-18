//Taken from Draw Stuff files provided by instructor
#ifndef _DRAW_STUFF_H_
#define _DRAW_STUFF_H_

//initialize the lcd
void DrawStuff_init();

//run the clean up code for lcd, turn it black
void DrawStuff_cleanup();

//runs the updates on the screen as required by program
void DrawStuff_updateScreen(char* message);

#endif
