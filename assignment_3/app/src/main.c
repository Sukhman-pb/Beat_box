#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "hal/i2c_bus.h"
#include "hal/helper.h"
// Read accelerometer X, Y, or Z axis (combines low and high bytes)
int16_t read_accelerometer_axis(int i2c_file_desc, uint8_t low_reg, uint8_t high_reg) {
    uint8_t low = read_register_8(i2c_file_desc, low_reg);
    uint8_t high = read_register_8(i2c_file_desc, high_reg);
    return (int16_t)(((high << 8) | low))>>2;  // Combine low and high bytes
}

// Enable accelerometer in normal mode
void enable_accelerometer(int i2c_file_desc) {
    uint8_t config_value = 0x50;  // ODR = 50 Hz, XYZ enabled (Refer to datasheet for control register settings)
    write_register_8(i2c_file_desc, REG_CTRL1, config_value);
}

int main() {
    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    enable_accelerometer(i2c_file_desc);
    int count = 0, sumX = 0, sumY = 0, sumZ = 0;
    while (1) {
        int16_t x = read_accelerometer_axis(i2c_file_desc, REG_OUT_X_L, REG_OUT_X_H);
        int16_t y = read_accelerometer_axis(i2c_file_desc, REG_OUT_Y_L, REG_OUT_Y_H);
        int16_t z = read_accelerometer_axis(i2c_file_desc, REG_OUT_Z_L, REG_OUT_Z_H);
        sumX +=abs(x);
        sumY +=abs(y);
        sumZ =+abs(z);
        count++;
        printf("Calibrated readings - X: %d, Y: %d, Z: %d\n", sumX/count, sumY/count, sumZ/count);
        sleep_ms(400);
    }
    // Close I2C Bus (this will never be reached unless loop is exited)
    close_i2c_bus(i2c_file_desc);
    printf("Accelerometer closed");

    return 0;
}

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hal/audioMixer.h"
#include "hal/helper.h"
#define BPM 120

// Calculate delay in milliseconds for half-beat
int getHalfBeatDelayMs() {
    return (60 * 1000) / (BPM * 2);
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

int main() {
    printf("Initializing Audio Mixer...\n");
    AudioMixer_init();

    // Load wave files
    wavedata_t basedrum, snare, hihat;
    AudioMixer_readWaveFileIntoMemory("wave-files/100051__menegass__gui-drum-bd-hard.wav", &basedrum);
    AudioMixer_readWaveFileIntoMemory("wave-files/100059__menegass__gui-drum-snare-soft.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("wave-files/100053__menegass__gui-drum-cc.wav", &hihat);

    int halfBeatDelayMs = getHalfBeatDelayMs();
    printf("Half-beat delay: %d ms at %d BPM\n", halfBeatDelayMs, BPM);

    // Play Prof's beat loop
    while (1) {
        playBeat(&basedrum, &snare, &hihat);
    }

    // Cleanup (unreachable unless you add a break condition)
    AudioMixer_freeWaveFileData(&basedrum);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&hihat);
    AudioMixer_cleanup();

    return 0;
}*/
