#include "audio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <math.h>

#define AUDIO_PWM_PIN 28
#define AUDIO_RATE 20000
#define AUDIO_WAVETABLE_SIZE 1000

static short int wavetable[AUDIO_WAVETABLE_SIZE];

static int step0 = 0;
static int offset0 = 0;
static int step1 = 0;
static int offset1 = 0;

static const int CHAN_AUDIO = AUDIO_PWM_PIN & 1U;
static const int SLICE_AUDIO = (AUDIO_PWM_PIN >> 1U) & 7U;

void init_wavetable() {
    for (int i = 0; i < AUDIO_WAVETABLE_SIZE; i++) {
        wavetable[i] =
            (16383 * sin(2 * M_PI * i / AUDIO_WAVETABLE_SIZE)) + 16384;
    }
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else {
            step0 = (f * AUDIO_WAVETABLE_SIZE / AUDIO_RATE) * (1 << 16);
        }
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else {
            step1 = (f * AUDIO_WAVETABLE_SIZE / AUDIO_RATE) * (1 << 16);
        }
    }
}

void audio_pwm_handler() {
    pwm_clear_irq(SLICE_AUDIO);

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= (AUDIO_WAVETABLE_SIZE << 16U)) {
        offset0 -= (AUDIO_WAVETABLE_SIZE << 16U);
    }
    if (offset1 >= (AUDIO_WAVETABLE_SIZE << 16U)) {
        offset1 -= (AUDIO_WAVETABLE_SIZE << 16U);
    }

    int samp = (wavetable[offset0 >> 16U] + wavetable[offset1 >> 16U]);
    samp >>= 1;
    samp = (samp * 49) / 32768;

    pwm_set_chan_level(SLICE_AUDIO, CHAN_AUDIO, samp);
}

void audio_init() {
    gpio_set_function(AUDIO_PWM_PIN, GPIO_FUNC_PWM);
    pwm_set_clkdiv(SLICE_AUDIO, 150.f);
    pwm_set_wrap(SLICE_AUDIO, (125000000 / 150 / AUDIO_RATE) - 1);
    pwm_set_chan_level(SLICE_AUDIO, CHAN_AUDIO, 0);

    init_wavetable();

    pwm_set_irq_enabled(SLICE_AUDIO, 1);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, audio_pwm_handler);
    irq_set_enabled(PWM_IRQ_WRAP, 1);

    pwm_set_enabled(SLICE_AUDIO, 1);
}

void audio_play_frequency(float freq) {
    set_freq(0, freq);
    set_freq(1, freq * 1.5f);
}

void audio_stop() {
    set_freq(0, 0.0f);
    set_freq(1, 0.0f);
}

static int victory_tune_note = -1;
static unsigned victory_tune_timer = 0;

// Optimized for piezo resonance (3-5kHz range)
// Classic "Power Up" victory tune in high octave
static float victory_notes[] = {
    2093.00f,  // C7 - quick ascending run
    2349.32f,  // D7
    2637.02f,  // E7
    3135.96f,  // G7
    4186.01f,  // C8 - triumphant high note!
    3135.96f,  // G7 - back down
    4186.01f,  // C8 - repeat for emphasis
    4186.01f   // C8 - hold for victory!
};
static int victory_durations[] = {
    120,  // Quick
    120,  // Quick
    120,  // Quick
    200,  // Medium
    300,  // Longer emphasis
    150,  // Quick
    200,  // Medium
    500   // Long victorious hold!
};
static int num_victory_notes = 8;

void audio_play_victory_tune() {
    victory_tune_note = 0;
    victory_tune_timer = time_us_32() / 1000;
    audio_play_frequency(victory_notes[0]);
}

void audio_update() {
    if (victory_tune_note < 0) {
        audio_stop();
        return;
    }
    uint32_t now = time_us_32() / 1000;
    
    if (now - victory_tune_timer >= victory_durations[victory_tune_note]) {
        victory_tune_note++;
        
        if (victory_tune_note < num_victory_notes) {
            audio_play_frequency(victory_notes[victory_tune_note]);
            victory_tune_timer = now;
        } else {
            audio_stop();
            victory_tune_note = -1;
        }
    }
}
