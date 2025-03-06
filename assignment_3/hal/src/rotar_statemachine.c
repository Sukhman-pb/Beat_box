#include "hal/rotar_statemachine.h"
#include "hal/gpio.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <gpiod.h>
#include <pthread.h>
//Basic idea taken from this link downlodable file with file name btn_statemachine.c https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/rotary_encoder/

#define GPIO_CHIP   GPIO_CHIP_2
#define GPIO_LINE_A  7
#define GPIO_LINE_B  8

//global variables
static bool is_initialized = false;
//line A and B
static struct GpioLine* s_lineA = NULL;
static struct GpioLine* s_lineB = NULL;
//atomic variable to store the counter
static atomic_int counter = 10;        //this is set to 10 as this value will be sent to the value frequency.
static atomic_int count_cw = 0;        //if this is 3 then only add to the counter i.e. counter clockwise
static atomic_int count_ccw = 0;       //if this is 3 then only subtract to the counter  i.e. counter clockwise                           
//thread to run the state machine
pthread_t rotary_thread;
//data type to store the next state and the action to be performed
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};
//data type to store the line events
struct state {
    struct stateEvent a_rising;
    struct stateEvent a_falling;
    struct stateEvent b_rising;
    struct stateEvent b_falling;
};

//after each counter increment reset the helper counters
static void reset_counter(void){
    count_cw = 0;
    count_ccw = 0;
}
//for each clockwise movement, we increase the helper clockwise counter
static void inc_clockwise(void){
    count_cw++;
}
//for each counter clockwise movement, we increase the helper counter clockwise counter
static void inc_counter_clockwise(void){
    count_ccw++;
}

//increase the counter by 1 if clockwise cycle is completed
static void rotary_clockwise(void){
    if(count_cw >= 3){
        if(counter<500){counter++;}
        else{atomic_store(&counter,500);}
        reset_counter();
    }
    //remove this else statement after testing
    /*else{
        printf("DEBUG: The count_cw was not properly set to 3 \n");
        printf("DEBUG: The value of count_cw is %d\n",count_cw);
        printf("DEBUG: The value of count_ccw is %d\n", count_ccw);
    }*/
}
//decrease the counter by 1 if counter clockwise cycle is completed.
static void rotary_counter_clockwise(void){
    if(count_ccw >= 3){
        if(counter>0)
        {counter--;}
        else
        {atomic_store(&counter, 0);}
        reset_counter();
    }
    //remove this else statement after testing 
    /*else{
        printf("DEBUG: The count_ccw was not set properly to 3 \n");
        printf("DEBUG: The value of count_cw is %d\n",count_cw);
        printf("DEBUG: The value of count_ccw is %d\n", count_ccw);
    }*/
    
}
//for every clockwise move, we change the values for the counter as control variables


//STATE MACHINE DESCRIPTION
struct state states[] = {
    {// 0
        .a_rising  = {&states[0], NULL},                        //on A rising stay in 0
        .a_falling = {&states[1], inc_clockwise},               //on A falling move from 0->1
        .b_rising  = {&states[0], NULL},                        //on B rising stay in 0 
        .b_falling = {&states[3], inc_counter_clockwise},       //on B falling move from 0->3
    },

    {// 1
        .a_rising  = {&states[0], rotary_counter_clockwise},    //on A rising move from 1->REST and counter - 1  
        .a_falling = {&states[1], NULL},                        //on A falling stay in 1
        .b_rising  = {&states[1], NULL},                        //on B rising stay in 1
        .b_falling = {&states[2], inc_clockwise},                        //on B falling move from 1->2   
    },
    {// 2
        .a_rising  = {&states[3], inc_clockwise},               //on A rising move from 2->3
        .a_falling = {&states[2], NULL},                        //on A falling stay in 2
        .b_rising  = {&states[1], inc_counter_clockwise},       //on B rising move from 2->1    
        .b_falling = {&states[2], NULL},                        //on B falling stay in 2
    },
    {// 3
        .a_rising  = {&states[3], NULL},                        //on A rising stay in 3 
        .a_falling = {&states[2], inc_counter_clockwise},                        //on A falling move from 3->2   
        .b_rising  = {&states[0], rotary_clockwise},            //on B rising move from 3->REST and counter + 1
        .b_falling = {&states[3], NULL},                        //on B falling stay in 3
    },
};

//this is the beginning state of the state machine
struct state* pCurrentState = &states[0];

//this is the void function that is called by the thread to run the state machine
void * rotar_thread(void* arg){
    (void)arg;
    while(is_initialized){
        rotar_state_machine_do_state();
    }
    return NULL;
}


void rotar_state_machine_init(){
    assert(!is_initialized);
    s_lineA = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_A);
    s_lineB = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_B);
    is_initialized = true;
    reset_counter();
    pthread_create(&rotary_thread, NULL, rotar_thread,NULL);
}


void rotar_state_machine_cleanup(){
    assert(is_initialized);
    is_initialized = false;
    Gpio_close(s_lineA);
    Gpio_close(s_lineB);
    pthread_cancel(rotary_thread);
    pthread_join(rotary_thread,NULL);
}


int rotar_state_machine_get_value(){
    return counter;
}


void set_counter(int value){
    atomic_store(&counter, value);
    //counter = value;         //this also works but atomic_store is better
}
//this function needs to be in a background thread therefre after the init function start this as a thread.
void rotar_state_machine_do_state(){
    assert(is_initialized);
    struct gpiod_line_bulk bulkEvents;
    int num_events = Gpio_waitForLineChange(s_lineA, s_lineB, &bulkEvents);
    for(int i = 0 ;i < num_events ; i++){
        //Get the line handle for this event
        struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

        //Ge tht number of this line
        unsigned int this_line_number = gpiod_line_offset(line_handle);

        //Get the line event
        struct gpiod_line_event event;
        if(gpiod_line_event_read(line_handle, &event) == -1){
            perror("Line Event");
            exit(EXIT_FAILURE);
        }
        
        //Run the state machine
        bool is_rising = (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE);
        //check for which line number the event is triggered
        bool is_A = (this_line_number == GPIO_LINE_A);
        bool is_B = (this_line_number == GPIO_LINE_B);

        assert(is_A || is_B);

        struct stateEvent* pEvent = NULL;
        if(is_A){// Process A rising or falling
                if(is_rising){
                    pEvent = &pCurrentState->a_rising;
                }
                else{
                    pEvent = &pCurrentState->a_falling;
                }    
        } 
        else if(is_B) {// Process B rising or falling
                if(is_rising){
                    pEvent = &pCurrentState->b_rising;
                }
                else{
                    pEvent = &pCurrentState->b_falling;
                }
        }
        // Execute the action if any
        if(pEvent->action != NULL) {
            //this is where we call the counter++ or counter -- if the action is not NULL
            pEvent->action();  
        }
        //change the current state to the next state
        pCurrentState = pEvent->pNextState;
    }
}