/*
 * Filename: main.c
 * Description: This program controls the BeagleY AI-based drum system, handling both hardware 
 * (joystick, accelerometer, knobs) and network-based (Node.js server) inputs to control 
 * beats, BPM, and volume. It also manages audio playback and display updates.
 * 
 * Author: Gurkirat Singh, Sukhman Singh
 * Date: 20, March 2025
 */

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
#include "hal/js_connection.h"
#include "hal/helper.h"

static int screen = 0;
static bool running = true;
static int beat_num = 0, bpm = 0;
static int count = 0;
// static int beat_num_ = 0, bpm_ = 0;
// static int count_ = 0;
int play = 3;
jsonCmd current = CMD_VOLUME;

static double minAudioLatency, maxAudioLatency, avgAudioLatency,  minAccelTiming, maxAccelTiming, avgAccelTiming;
static pthread_mutex_t update_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_t lcd, audio_thread, accel_audio_thread, timer, from_thread, to_thread, play_thread;

// ** Declare Audio WaveData **
static wavedata_t basedrum1, snare1, hihat1, basedrum, snare, hihat, x_sound, y_sound, z_sound;

void* from_update_thread(void* arg) {
    (void)arg;
    while(current != CMD_STOP){

        pthread_mutex_lock(&update_lock);
        count = joystick_getVol();
        AudioMixer_setVolume(count);
        //pthread_mutex_unlock(&update_lock1);

        //pthread_mutex_lock(&update_lock2);
        beat_num = knob_getBeatNum();
        //pthread_mutex_unlock(&update_lock2);

        //pthread_mutex_lock(&update_lock3);
        bpm = knob_getBPM();  
        pthread_mutex_unlock(&update_lock);

        jSconnection_fromMain(&count, &bpm, &beat_num, &play);    
    }
    return NULL;
}

void* to_update_thread(void* arg) {
    (void)arg;
    bool isfirst = true;
    while(current != CMD_STOP){
        
        jSconnection_toMain(&count, &bpm, &beat_num, &play);
        
        //AudioMixer_setVolume(count_);
        pthread_mutex_lock(&update_lock);
        if(isfirst == false){joystick_setVol(count);}
        //pthread_mutex_unlock(&update_lock1);

        //pthread_mutex_lock(&update_lock2);
        knob_setBeatNum(beat_num);
        //pthread_mutex_unlock(&update_lock2);

        //pthread_mutex_lock(&update_lock3);
        knob_setBPM(bpm); 
        pthread_mutex_unlock(&update_lock);

        isfirst = false;
    }
return NULL;
}

/*
 * Function: timer_update_thread
 * -----------------------------
 * Periodically collects and prints latency statistics for audio and accelerometer events.
 * Helps in debugging system timing and performance.
 */

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
        printf("M%d  %dbpm  vol:%d   Audio[%.3f, %.3f] avg %.3f/%d   Accel[%.3f, %.3f] avg %.3f/%d\n\n",
            beat_num, bpm, count, 
            minAudioLatency, maxAudioLatency, avgAudioLatency, audioStats.numSamples, 
            minAccelTiming, maxAccelTiming, avgAccelTiming, accelStats.numSamples);

        sleep_ms(1000);  // Print every second
    }
    return NULL;
}


void* play_audio_thread_func(void* arg) {
    (void)arg;

    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &basedrum1);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav", &snare1);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100053__menegass__gui-drum-cc.wav", &hihat1);

    while (running) {
        
        if (play == 1 ) {
            AudioMixer_queueSound(&hihat1);
            play = 3;
        }
        if (play == 2) {
            AudioMixer_queueSound(&snare1);
            play = 3;
        }
        if (play == 0) {
            AudioMixer_queueSound(&basedrum1);
            play = 3;
        }

    }
    AudioMixer_freeWaveFileData(&basedrum1);
    AudioMixer_freeWaveFileData(&snare1);
    AudioMixer_freeWaveFileData(&hihat1);
    return NULL;
}

/*
 * Function: accel_audio_thread_func
 * Monitors accelerometer movements and queues corresponding drum sounds.
 * Detects movement along X, Y, and Z axes and plays preloaded audio samples.
 */

void* accel_audio_thread_func(void* arg) {
    (void)arg;

    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &x_sound);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav", &y_sound);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100053__menegass__gui-drum-cc.wav", &z_sound);

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

        sleep_ms(10);  // Short delay 
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

/*
 * Function: playBeat
 * Plays a predefined drum beat pattern using the loaded audio samples.
 * Ensures beats are queued at correct timing intervals.
 */

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

    
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &basedrum);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100053__menegass__gui-drum-cc.wav", &hihat);

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


/*
 * Function: lcd_update_thread
 * Updates the LCD display with the current beat, BPM, and audio timing statistics.
 * Cycles through different screens displaying system status.
 */

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

/*
 * Function: main
 * Initializes all peripherals (joystick, knobs, accelerometer, network, and audio).
 * Starts background threads for audio playback, network communication, and display updates.
 * Continuously processes input commands until a stop command is received.
 * Cleans up all resources in the reverse order of initialization.
 */

int main(){
    Period_init();
    DrawStuff_init();
    Gpio_initialize();
    int i2c_file_desc = joystick_init();
    knob_init();
    accel_init();
    
    // ** Initialize Audio and everything **
    AudioMixer_init();
    pthread_create(&lcd, NULL, lcd_update_thread, NULL);
    pthread_create(&timer, NULL, timer_update_thread, NULL);
    pthread_create(&audio_thread, NULL, audio_play_thread, NULL);
    pthread_create(&accel_audio_thread, NULL, accel_audio_thread_func, NULL);
    pthread_create(&play_thread, NULL, play_audio_thread_func, NULL);
    jSconnection_init();
    pthread_create(&from_thread, NULL, from_update_thread, NULL);
    pthread_create(&to_thread, NULL, to_update_thread, NULL);
    
    count = joystick_getVol();
    AudioMixer_setVolume(count);
    beat_num = knob_getBeatNum();
    bpm = knob_getBPM();
    do {
        screen = joystick_getScreenCount();
        printf("Volume %d  BPM %d  Mode %d  playSound %d \n", count, bpm, beat_num, play);

        current = jSconnection_getCurrentCommand(); // Read current command
        sleep_ms(100);
        
    } while (current != CMD_STOP); // Exit only when stop command is received


    pthread_join(to_thread, NULL);
    pthread_join(from_thread, NULL);

    // Stop network connection cleanly
    jSconnection_stop();

    // Stop audio processing
    running = false;
    pthread_join(play_thread, NULL);
    pthread_join(accel_audio_thread, NULL);
    pthread_join(audio_thread, NULL);

    printf("JS UDP connection stopped.\n");
    // Stop timer and LCD
    pthread_join(timer, NULL);
    pthread_join(lcd, NULL);

    joystick_cancel();
    knob_cancel();

    knob_close();
    accel_close();
    joystick_close(i2c_file_desc);
    Gpio_cleanup();
    DrawStuff_cleanup();
    AudioMixer_cleanup();
    Period_cleanup();
    return 0;
}

