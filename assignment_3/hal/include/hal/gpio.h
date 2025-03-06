// Low-level GPIO access using gpiod
//this file is provided by the professor and it is used as is 
//I have added a linne in here in order to read from two channels at the same time
//We need to read chip 2 line 7 and 8 at the same time
//taken from this link downlodable file with file name gpio.h https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/rotary_encoder/
#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdbool.h>
#include <gpiod.h>

// Opaque structure
struct GpioLine;

enum eGpioChips {
    GPIO_CHIP_0,
    GPIO_CHIP_1,
    GPIO_CHIP_2,
    GPIO_NUM_CHIPS      // Count of chips
};

// Must initialize before calling any other functions.
void Gpio_initialize(void);
void Gpio_cleanup(void);


// Opening a pin gives us a "line" that we later work with.
//  chip: such as GPIO_CHIP_0
//  pinNumber: such as 15
struct GpioLine* Gpio_openForEvents(enum eGpioChips chip, int pinNumber);

int Gpio_waitForLineChange(
    struct GpioLine* line1,
    struct GpioLine* line2, 
    struct gpiod_line_bulk *bulkEvents
);

void Gpio_close(struct GpioLine* line);

#endif