/**
Authors
Sukhman Singh
Gurkirat Singh
Assignment 2 (433)
Main file which runs and reads values about dips, frequency of the emitter, and stats of the samples form other files and displays it
on the terminal each second and also updates the screen connected to the Beagle y through the separate thread. 
function sleeps for one second before taking another set of values 
it also initializes all the helper and hal modules ione by one 
at the end, it closes all the hal modules and free the memory
    */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>  // For sleep()
#include "draw_stuff.h"
#include "hal/sampler.h"
#include "hal/rotar_statemachine.h"
#include "hal/gpio.h"
#include "hal/pwm.h"
#include "hal/periodTimer.h"
#include "hal/connection.h"
#include "hal/helper.h"
#include <pthread.h>
#include <mcheck.h>

// Shared variables for all the threads and main function
static pthread_t lcdThread;
static bool lcdRunning = true;
static pthread_mutex_t lcdMutex = PTHREAD_MUTEX_INITIALIZER;
int dip_count=0, sample_rate = 0, flash_rate = 0, history_second_size = 0;
int lcd_flash_rate = 0;    //shared variable for LCD thread
int lcd_dip_count = 0;     //""
double lcd_max_ms = 0.0;   //""
double avg_voltage = 0.0;
long long total = 0;
double* history_second= NULL;

void printSampledValues() {
    int historySize = 0;
    double *samples = Sampler_getHistory(&historySize);

    if (samples == NULL || historySize == 0) {
        //printf("No samples available from the previous second.\n");
        return;
    }

    int numSamplesToDisplay = (historySize < 10) ? historySize : 10;
    int step = (historySize < 10) ? 1 : historySize / 10; // Even spacing

    for (int i = 0; i < numSamplesToDisplay; i++) {
        int index = i * step; 
        if (index >= historySize) { // makee sure we don’t exceed bounds
            index = historySize - 1;
        }
        printf("%d:%.3f  ", index, samples[index]);
    }

    printf("\n\n");
    free(samples); // Free allocated memory from Sampler_getHistory()
}

// Thread function for updating the LCD
void* lcd_update_thread(void* arg) {
    (void)arg;
    char lcd_buff[128];

    while (lcdRunning) {
        pthread_mutex_lock(&lcdMutex);  // Lock before reading shared data
        snprintf(lcd_buff, sizeof(lcd_buff),
                 "Sukhman Gurkirat!\n"
                 "Flash @ %2dHz\n"
                 "Dips  = %3d\n"
                 "Max ms: %.1f",
                 lcd_flash_rate, lcd_dip_count, lcd_max_ms);
        pthread_mutex_unlock(&lcdMutex);  // Unlock after reading

        DrawStuff_updateScreen(lcd_buff);
        sleep_ms(1000);
    }
    return NULL;
}


int main() {   
    // Initialize LCD and Sampler
    Period_init();
    Gpio_initialize();
    DrawStuff_init();
    Sampler_init();
    init_pwm();
    rotar_state_machine_init();
    start_connection();
    

    sleep(1);  // Give time for initialization

    pthread_create(&lcdThread, NULL, lcd_update_thread, NULL);

    char term_buff[128];  // Buffer for Terminal Output
    Period_statistics_t periodStats;
    Command_type current = get_current_command();    // Get the current command, if stop then exit otherwise run
    while (current != CMD_STOP) {
        //gets the counter to set the frequency
        flash_rate = rotar_state_machine_get_value(); 
        change_frequency(flash_rate);
        dip_count = Sampler_getDipCounter();  // Number of dips detected
        Sampler_moveCurrentDataToHistory();     // Move the current data to history and set the dips to 0 so we need to store the dips before it
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &periodStats);
        sample_rate = Sampler_getHistorySize();  // Number of light samples in the last second
        avg_voltage = Sampler_getAverageReading();  // Average voltage (EMA)
        //check if it works without mutexes
        pthread_mutex_lock(&lcdMutex);
        lcd_flash_rate = flash_rate;
        lcd_dip_count = dip_count;
        lcd_max_ms = periodStats.maxPeriodInMs;
        pthread_mutex_unlock(&lcdMutex);

        snprintf(term_buff, sizeof(term_buff),
                 "#Smpl/s = %d  Flash @ %dHz  avg = %.3fV  dips = %d  "
                 "Smpl ms [%.3f, %.3f]  avg = %.3f/%d",
                 sample_rate, flash_rate, avg_voltage, dip_count,
                 periodStats.minPeriodInMs, periodStats.maxPeriodInMs,
                 periodStats.avgPeriodInMs, sample_rate);

        printf("%s\n", term_buff);

        printSampledValues();
        current = get_current_command();//CHECK FOR EXIT CONDITION
        total = Sampler_getNumSamplesTaken();
        history_second = Sampler_getHistory(&history_second_size);
        //printf("The size of the history is %d\n", history_second_size);
        update(total, dip_count, sample_rate, history_second, history_second_size);
        // Sleep for 1 second before updating again
        sleep_ms(1000);
        //freeing the pointer to the history as on every call it is assigned a new memory location
        free(history_second);
    }

    //cleanup
    lcdRunning = false;
    pthread_join(lcdThread, NULL);
    DrawStuff_cleanup();
    Sampler_cleanup();
    rotar_state_machine_cleanup();
    Gpio_cleanup();
    close_pwm();
    Period_cleanup();
    stop_connection();
    
    printf("!!! DONE !!!\n");
    return 0;
}
