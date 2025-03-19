#include "hal/knob.h"
#include "hal/btn_statemachine.h"
#include "hal/rotar_statemachine.h"
#include "pthread.h"
#include "assert.h"

pthread_t button1,rotar;
static bool isInitialized = false;

void* buttonThread(void* arg){
    (void) arg;
    while(1){
        BtnStateMachine_rotary();
    }
    return NULL;
}

void* rotaryThread(void* arg){
    (void) arg;
    while(1){
        rotar_state_machine_do_state();
    }
    return NULL;
}
void knob_init(void){
    assert(!isInitialized);
    BtnStateMachine_rotaryInit();
    rotar_state_machine_init();
    isInitialized = true;
    //add threads for both the rotary encoder functionalities here
    pthread_create(&button1 ,NULL, buttonThread, NULL);
    pthread_create(&rotar, NULL, rotaryThread, NULL);

}

void knob_cancel(){
    pthread_cancel(button1);
    pthread_cancel(rotar);
}
int knob_getBeatNum(){
    return BtnStateMachine_getValue()%3;
}
int knob_getBPM(){
    return rotar_state_machine_get_value();
}
void knob_close(void){
    assert(isInitialized);
    pthread_join(button1, NULL);
    pthread_join(rotar,NULL);
    BtnStateMachine_RotBtnCleanup();
    rotar_state_machine_cleanup();
}