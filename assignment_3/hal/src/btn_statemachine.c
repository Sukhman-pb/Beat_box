// Sample state machine for one GPIO pin.

#include "hal/btn_statemachine.h"
#include "hal/gpio.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define ROTARY_CHIP          GPIO_CHIP_0
#define JOYSTICK_CHIP      GPIO_CHIP_2
#define ROTARY_ENCODER_PIN   10
#define JOYSTICK_PIN       15


static bool isInitialized = false;

struct GpioLine* s_lineBtn = NULL;
struct GpioLine* s_lineJoy = NULL;
static atomic_int counter_rotary = 0;
static atomic_bool joyIsPressed = false;
static atomic_int general_counter = 0;
/*
    Define the Statemachine Data Structures
*/
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};
struct state {
    struct stateEvent rising;
    struct stateEvent falling;
};


/*
    START STATEMACHINE
*/
static void onRelease()
{
    counter_rotary++;
}

static void onPressJoy()
{
    joyIsPressed = true;
    general_counter++;
}


struct state states[] = {
    { // Not pressed
        .rising = {&states[0], NULL},
        .falling = {&states[1], onRelease},
    },

    { // Pressed
        .rising = {&states[0], NULL},
        .falling = {&states[1], NULL},
    },
};

struct state statesJoy[] = {
    { // Not pressed
        .rising = {&statesJoy[0], NULL},  //sets the JoyisPressed to false
        .falling = {&statesJoy[1], onPressJoy},
    },

    { // Pressed
        .rising = {&statesJoy[0], NULL},
        .falling = {&statesJoy[1], NULL},
    },
};

/*
    END STATEMACHINE
*/

struct state* pCurrentState = &states[0];
struct state* pCurrentStateJoy = &statesJoy[0];

void BtnStateMachine_init()
{
    assert(!isInitialized);
    s_lineBtn = Gpio_openForEvents(ROTARY_CHIP,  ROTARY_ENCODER_PIN);
    s_lineJoy = Gpio_openForEvents(JOYSTICK_CHIP, JOYSTICK_PIN);
    isInitialized = true;
}
void BtnStateMachine_cleanup()
{
    assert(isInitialized);
    isInitialized = false;
    Gpio_close(s_lineBtn);
    Gpio_close(s_lineJoy);
}

int BtnStateMachine_getValue()
{
    //keeps the count of the counters in the range of 0 to 2
    return counter_rotary;
}
bool BtnStateMachine_isPressed(){
    return joyIsPressed;
}

int BtnStateMachine_counterPressed(){
    return general_counter;
}

// TODO: This should be on a background thread!
void BtnStateMachine_rotary()
{
    assert(isInitialized);

    //printf("\n\nWaiting for an event...\n");
    // while (true) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChangeSingle(s_lineBtn, &bulkEvents);

        // Iterate over the event
        for (int i = 0; i < numEvents; i++)
        {
            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle,&event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }


            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool isBtn = this_line_number == ROTARY_ENCODER_PIN;
            assert (isBtn);

            struct stateEvent* pStateEvent = NULL;
            if (isRising) {
                pStateEvent = &pCurrentState->rising;
            } else {
                pStateEvent = &pCurrentState->falling;
            } 
            // Do the action
            if (pStateEvent->action != NULL) {
                pStateEvent->action();
            }
            pCurrentState = pStateEvent->pNextState;
            

            // DEBUG INFO ABOUT STATEMACHINE
            #if 0
            int newState = (pCurrentState - &states[0]);
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s -> new state %d     [%f]\n", 
                i, 
                numEvents,
                this_line_number, 
                isRising ? "RISING": "falling", 
                newState,
                time);
            #endif
        }
    // }

}

void BtnStateMachine_joystick()
{
    assert(isInitialized);

    //printf("\n\nWaiting for an event...\n");
    // while (true) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChangeSingle(s_lineJoy, &bulkEvents);

        // Iterate over the event
        for (int i = 0; i < numEvents; i++)
        {
            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle,&event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }


            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool isBtn = this_line_number == JOYSTICK_PIN;
            assert (isBtn);

            struct stateEvent* pStateEvent = NULL;
            if (isRising) {
                pStateEvent = &pCurrentStateJoy->rising;
            } else {
                pStateEvent = &pCurrentStateJoy->falling;
            } 
            // Do the action
            if (pStateEvent->action != NULL) {
                pStateEvent->action();
            }
            pCurrentStateJoy = pStateEvent->pNextState;

            // DEBUG INFO ABOUT STATEMACHINE
            #if 0
            int newState = (pCurrentState - &states[0]);
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s -> new state %d     [%f]\n", 
                i, 
                numEvents,
                this_line_number, 
                isRising ? "RISING": "falling", 
                newState,
                time);
            #endif
        }
    // }

}
