#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// Audio configuration
#define AUDIO_PWM_PIN 36
#define AUDIO_RATE 20000  // Sample rate in Hz
#define AUDIO_WAVETABLE_SIZE 1000

// Audio functions
void audio_init(void);
void audio_play_frequency(float freq);
void audio_stop(void);
void audio_play_victory_tune(void);

#endif // AUDIO_H
