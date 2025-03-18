#include <stdio.h>
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
}