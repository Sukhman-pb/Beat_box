#include "hal/joy_stick_control.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include "hal/helper.h"
#include "hal/rotar_statemachine.h"
#include "hal/gpio.h"
#include "hal/knob.h"
#include "draw_stuff.h"
#include "hal/audioMixer.h"
#include "hal/accel.h"

static int screen = 0;
static bool running = true;
static int beat_num = 0, bpm = 0;
static int count = 0;
static double minAudioLatency, maxAudioLatency, avgAudioLatency,  minAccelTiming, maxAccelTiming, avgAccelTiming;
//static pthread_mutex_t data = PTHREAD_MUTEX_INITIALIZER;

static pthread_t lcd, audio_thread, accel_audio_thread, timer;

// ** Declare Audio WaveData **
static wavedata_t basedrum, snare, hihat;


static wavedata_t x_sound, y_sound, z_sound;



void* timer_update_thread(void* arg) {
    (void)arg;


    Period_statistics_t audioStats;
    Period_statistics_t accelStats;

    while (running) {
        // Get timing statistics for audio playback
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_PLAYBACK_AUDIO, &audioStats);
        minAudioLatency = audioStats.minPeriodInMs;
        maxAudioLatency = audioStats.maxPeriodInMs;
        avgAudioLatency = audioStats.avgPeriodInMs;

        // Get timing statistics for accelerometer
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_ACCEL, &accelStats);
        minAccelTiming = accelStats.minPeriodInMs;
        maxAccelTiming = accelStats.maxPeriodInMs;
        avgAccelTiming = accelStats.avgPeriodInMs;

        // Print formatted statistics
        printf("M%d %dbpm vol:%d   Audio[%.3f, %.3f] avg %.3f/%d   Accel[%.3f, %.3f] avg %.3f/%d\n\n",
            beat_num, bpm, count, 
            minAudioLatency, maxAudioLatency, avgAudioLatency, audioStats.numSamples, 
            minAccelTiming, maxAccelTiming, avgAccelTiming, accelStats.numSamples);

        sleep_ms(1000);  // Print every second
    }

    return NULL;
}




// ** Accelerometer Thread Function **
void* accel_audio_thread_func(void* arg) {
    (void)arg;

    
    AudioMixer_readWaveFileIntoMemory("wave-files/100051__menegass__gui-drum-bd-hard.wav", &x_sound);
    AudioMixer_readWaveFileIntoMemory("wave-files/100059__menegass__gui-drum-snare-soft.wav", &y_sound);
    AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &z_sound);

    while (running) {
        bool x = get_accel(1);
        bool y = get_accel(2);
        bool z = get_accel(3);
        //printf("X: %d, Y: %d, Z: %d -------------- \n", x, y, z);
        if (x) {
            //printf("X-Axis Movement Detected! Playing sound...\n");
            AudioMixer_queueSound(&x_sound);
        }
        if (y) {
            //printf("Y-Axis Movement Detected! Playing sound...\n");
            AudioMixer_queueSound(&y_sound);
        }
        if (z) {
            //printf("Z-Axis Movement Detected! Playing sound...\n");
            AudioMixer_queueSound(&z_sound);
        }

        sleep_ms(10);  // Short delay to avoid spamming
    }
    AudioMixer_freeWaveFileData(&x_sound);
    AudioMixer_freeWaveFileData(&y_sound);
    AudioMixer_freeWaveFileData(&z_sound);
    return NULL;
}


// Calculate delay in milliseconds for half-beat
int getHalfBeatDelayMs() {
    return (60 * 1000) / (bpm * 2);
}

void playBeat(wavedata_t *basedrum, wavedata_t *snare, wavedata_t *hihat) {
    int halfBeatDelayMs = getHalfBeatDelayMs();

    // Beat 1: Hi-hat, Base
    AudioMixer_queueSound(hihat);
    AudioMixer_queueSound(basedrum);
    sleep_ms(halfBeatDelayMs);

    // Beat 1.5: Hi-hat
    AudioMixer_queueSound(hihat);
    sleep_ms(halfBeatDelayMs);

    // Beat 2: Hi-hat, Snare
    AudioMixer_queueSound(hihat);
    AudioMixer_queueSound(snare);
    sleep_ms(halfBeatDelayMs);

    // Beat 2.5: Hi-hat
    AudioMixer_queueSound(hihat);
    sleep_ms(halfBeatDelayMs);

    // Beat 3: Hi-hat, Base
    AudioMixer_queueSound(hihat);
    AudioMixer_queueSound(basedrum);
    sleep_ms(halfBeatDelayMs);

    // Beat 3.5: Hi-hat
    AudioMixer_queueSound(hihat);
    sleep_ms(halfBeatDelayMs);

    // Beat 4: Hi-hat, Snare
    AudioMixer_queueSound(hihat);
    AudioMixer_queueSound(snare);
    sleep_ms(halfBeatDelayMs);

    // Beat 4.5: Hi-hat
    AudioMixer_queueSound(hihat);
    sleep_ms(halfBeatDelayMs);
}

// ** Audio Thread Function **
void* audio_play_thread(void* arg) {
    (void)arg;

    
    AudioMixer_readWaveFileIntoMemory("wave-files/100051__menegass__gui-drum-bd-hard.wav", &basedrum);
    AudioMixer_readWaveFileIntoMemory("wave-files/100059__menegass__gui-drum-snare-soft.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &hihat);

    while (running) {
        if (beat_num == 0) {
            //sleep_ms(100);

        } else if (beat_num == 1) {
            // Play Prof's beat
            playBeat(&basedrum, &snare, &hihat);
        } else if (beat_num == 2) {
            
            playBeat(&hihat, &snare, &basedrum);
            //sleep_ms(100);
        }
    }

    // ** Free Audio Resources **
    AudioMixer_freeWaveFileData(&basedrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&hihat);

    return NULL;
}



void* lcd_update_thread(void* arg){
    (void)arg;
    char lcdBuff[264];
    while (running) {
        switch (screen) {
            case 0:  // Screen 1: Status screen
                snprintf(lcdBuff, sizeof(lcdBuff),
                    "    Beat %d\n"
                    "      \n"
                    "      \n"
                    "      \n"
                    "      \n"
                    "      \n"
                    "Volume %3d    "
                    "BPM %3d\n",
                    beat_num, count, bpm);
                break;

            case 1:  // Screen 2: Audio Timing Summary
                snprintf(lcdBuff, sizeof(lcdBuff),
                    " Audio Timing\n"
                    "      \n"
                    " ------------------- \n"
                    "      \n"
                    " Min:  %.3f ms\n"
                    " Max:  %.3f ms\n"
                    " Avg:  %.3f ms\n"
                    "      \n",
                
                    minAudioLatency, maxAudioLatency, avgAudioLatency);  
                break;

            case 2:  // Screen 3: Accelerometer Timing
                snprintf(lcdBuff, sizeof(lcdBuff),
                    " Accel.Timing\n"
                    "      \n"
                    " ------------------- \n"
                    "      \n"
                    " Min:  %.3f ms\n"
                    " Max:  %.3f ms\n"
                    " Avg:  %.3f ms\n"
                    "      \n",

                    minAccelTiming, maxAccelTiming, avgAccelTiming);  
                break;
        }

        DrawStuff_updateScreen(lcdBuff);
    }
    return NULL;
}
int main(){
    Period_init();
    DrawStuff_init();
    Gpio_initialize();
    int i2c_file_desc = joystick_init();
    knob_init();
    accel_init();
    
    // ** Initialize Audio **
    AudioMixer_init();
    pthread_create(&lcd, NULL, lcd_update_thread, NULL);
    pthread_create(&timer, NULL, timer_update_thread, NULL);
    pthread_create(&audio_thread, NULL, audio_play_thread, NULL);
    pthread_create(&accel_audio_thread, NULL, accel_audio_thread_func, NULL);
    int shutdown_ = 0;
  
    while(1){
        count = joystick_getVol();
        //printf("volume is :%d\n", count);
        AudioMixer_setVolume(count);
        screen = joystick_getScreenCount();
        //printf("Screen number is :%d\n", screen);
        beat_num = knob_getBeatNum();
        //printf("The beat to play is :%d\n", beat_num);

        bpm = knob_getBPM();
        //printf("The beat per minute is:%d\n", bpm);
        sleep_ms(50);
        shutdown_ ++;
        if(shutdown_ == 2000){
            joystick_cancel();
            knob_cancel();
            running = false;
            break;}
    }
    knob_close();
    accel_close();
    joystick_close(i2c_file_desc);
    Gpio_cleanup();
    DrawStuff_cleanup();
    AudioMixer_cleanup();
    pthread_join(accel_audio_thread, NULL);
    pthread_join(timer, NULL);
    pthread_join(lcd, NULL);
    pthread_join(audio_thread, NULL);
    Period_cleanup();
    return 0;
}