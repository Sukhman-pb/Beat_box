#include "hal/sampler.h"
#include "hal/i2c_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "hal/helper.h"
#include <time.h>

#include <stdatomic.h>

static double currentBuffer[1000];  // Current second samples
static double historyBuffer[1000];  // Last second samples
static int currentSampleCount = 0;         // Number of samples in current second
static int historySampleCount = 0;         // Number of samples in last second

static int dip_counter = 0;
static pthread_t samplerThread;
static pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dipMutex = PTHREAD_MUTEX_INITIALIZER;
static bool isRunning = false;

static double currentAverage = 0.0;
static bool isFirstSample = true;
static int i2c_file_desc;
static long long totalSamples = 0;
static bool dipDetected = false;  // Tracks if a dip has been detected

static void updateAverage(double sample) {
    if (isFirstSample) {
        currentAverage = sample;
        isFirstSample = false;
    } else {
        currentAverage = (.001 * sample) + (.999 * currentAverage);
    }
     //printf("DEBUG: sample = %.6f, currentAverage = %.6f\n", sample, currentAverage);
}

static void storeSample(double sample) {
    pthread_mutex_lock(&bufferMutex);
    if (currentSampleCount < 1000) {
        currentBuffer[currentSampleCount++] = sample;
    }
    pthread_mutex_unlock(&bufferMutex);
}

static void detectDip(double sample) {
    double dipThreshold = currentAverage * 0.9;   // 0.1V below average
    double resetThreshold = currentAverage * 0.93; // 0.07V below average for reset

    if (!dipDetected && sample <= dipThreshold) {
        pthread_mutex_lock(&dipMutex);
        dip_counter++;
        pthread_mutex_unlock(&dipMutex);
        dipDetected = true;  // Prevent immediate retrigger
    }

    // Reset dip detection when light level rises above reset threshold
    if (dipDetected && sample >= resetThreshold) {
        dipDetected = false;
    }
}


static void* read_voltage(void* arg) {
    (void)arg;
    while (isRunning) {
        float sample = getVoltage( i2c_file_desc );    // Read from ADC
        
        Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
        totalSamples++;                        // Count total samples
        updateAverage(sample);                // Update EMA
        storeSample(sample);
        detectDip(sample);  // Check if a dip occurred
        sleep_ms(1);                        // Sleep 1 millisecond
    }
    return NULL;
}
// Begin/end the background thread which samples light levels.
void Sampler_init(void){
    i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, TLA_CHANNEL_CONF_2);
    isRunning = true;
    pthread_create(&samplerThread, NULL, read_voltage, NULL);
}
void Sampler_cleanup(void){
    isRunning = false;
    pthread_join(samplerThread, NULL);
    close_i2c_bus(i2c_file_desc);
    pthread_mutex_lock(&bufferMutex);
    memset(currentBuffer, 0, sizeof(currentBuffer));  // Clear current buffer
    memset(historyBuffer, 0, sizeof(historyBuffer));  // Clear history buffer
    historySampleCount = 0;
    currentSampleCount = 0;
    pthread_mutex_unlock(&bufferMutex);
    pthread_mutex_destroy(&bufferMutex);
    pthread_mutex_destroy(&dipMutex);
}
// Must be called once every 1s.
// Moves the samples that it has been colleorycting this second into
// the history, which makes the samples available for reads (below).
void Sampler_moveCurrentDataToHistory(void) {
    pthread_mutex_lock(&bufferMutex);
    memcpy(historyBuffer, currentBuffer, currentSampleCount * sizeof(double));
    
    historySampleCount = currentSampleCount;
    currentSampleCount = 0;  // Reset current buffer for the next second
    pthread_mutex_lock(&dipMutex);
    dip_counter = 0;
    pthread_mutex_unlock(&dipMutex);
    pthread_mutex_unlock(&bufferMutex);
}



// Get the number of samples collected during the previous complete second.
int Sampler_getHistorySize(void) {
    int size = 0;
    pthread_mutex_lock(&bufferMutex);
    size = historySampleCount;
    pthread_mutex_unlock(&bufferMutex);
    return size;
}
// Get a copy of the samples in the sample history.
// Returns a newly allocated array and sets `size` to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
// Note: It provides both data and size to ensure consistency.
double* Sampler_getHistory(int *size){
    pthread_mutex_lock(&bufferMutex);
    *size = historySampleCount;
    double* copy = malloc(sizeof(double) * historySampleCount);
    if (copy) {
        memcpy(copy, historyBuffer, sizeof(double) * historySampleCount);
    }
    //historySampleCount = 0;
    pthread_mutex_unlock(&bufferMutex);
    return copy;
}


// Get the average light level (not tied to the history).
double Sampler_getAverageReading(void) {
    return currentAverage;
}
// Get the total number of light level samples taken so far.
long long Sampler_getNumSamplesTaken(void) {
    return totalSamples;
}
int Sampler_getDipCounter(void){
    int value;
    pthread_mutex_lock(&dipMutex);
    value = dip_counter;
    pthread_mutex_unlock(&dipMutex);
    return value;
}
