#include "audio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <math.h>

#define AUDIO_PWM_PIN 28
#define AUDIO_RATE 20000
#define AUDIO_WAVETABLE_SIZE 1000

#define AUDIO_CHANNEL (AUDIO_PWM_PIN & 1U)
#define AUDIO_SLICE ((AUDIO_PWM_PIN >> 1U) & 7U)

static short wavetable[AUDIO_WAVETABLE_SIZE];

static void init_wavetable();
static void set_freq(int chan, float f);
static void audio_pwm_handler();

static int step0, step1;
static int offset0, offset1;

static int victory_tune_note = -1;
static unsigned victory_tune_timer = 0;
static float victory_notes[] = {2093.00f, 2349.32f, 2637.02f, 3135.96f, 4186.01f, 3135.96f, 4186.01f, 4186.01f};
static int victory_durations[] = {120, 120, 120, 200, 300, 150, 200, 500};
static int num_victory_notes = 8;

static int game_start_note = -1;
static unsigned game_start_timer = 0;
static float game_start_notes[] = {2637.02f, 3135.96f, 4186.01f};
static int game_start_durations[] = {150, 150, 400};
static int num_game_start_notes = 3;

static bool blip_playing = false;
static unsigned blip_timer = 0;

void audio_init() {
    gpio_set_function(AUDIO_PWM_PIN, GPIO_FUNC_PWM);
    pwm_set_clkdiv(AUDIO_SLICE, 150.f);
    pwm_set_wrap(AUDIO_SLICE, (125000000 / 150 / AUDIO_RATE) - 1);
    pwm_set_chan_level(AUDIO_SLICE, AUDIO_CHANNEL, 0);

    init_wavetable();

    pwm_set_irq_enabled(AUDIO_SLICE, 1);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, audio_pwm_handler);
    irq_set_enabled(PWM_IRQ_WRAP, 1);

    pwm_set_enabled(AUDIO_SLICE, 1);
}

void audio_update() {
    const uint32_t now_ms = time_us_32() / 1000;

    if (blip_playing) {
        uint32_t now = time_us_32() / 1000;
        
        if (now - blip_timer >= 15) {
            audio_stop();
            blip_playing = false;
        }

    } else if (game_start_note >= 0) {
        if (now_ms - game_start_timer >= game_start_durations[game_start_note]) {
            game_start_note++;

            if (game_start_note < num_game_start_notes) {
                audio_play_frequency(game_start_notes[game_start_note]);
                game_start_timer = now_ms;
            } else {
                audio_stop();
                game_start_note = -1;
            }
        }

    } else if (victory_tune_note >= 0) {
        if (now_ms - victory_tune_timer >= victory_durations[victory_tune_note]) {
            victory_tune_note++;
            
            if (victory_tune_note < num_victory_notes) {
                audio_play_frequency(victory_notes[victory_tune_note]);
                victory_tune_timer = now_ms;
            } else {
                audio_stop();
                victory_tune_note = -1;
            }
        }

    } else {
        audio_stop();
    }
}

void audio_stop() {
    set_freq(0, 0.0f);
    set_freq(1, 0.0f);
}

void audio_play_frequency(float freq) {
    set_freq(0, freq);
    set_freq(1, freq * 1.5f);
}

void audio_play_victory_tune() {
    victory_tune_note = 0;
    victory_tune_timer = time_us_32() / 1000;
    audio_play_frequency(victory_notes[0]);
}

void audio_play_blip() {
    blip_playing = true;
    blip_timer = time_us_32() / 1000;
    audio_play_frequency(3520.0f);
}

void audio_play_game_start() {
    game_start_note = 0;
    game_start_timer = time_us_32() / 1000;
    audio_play_frequency(game_start_notes[0]);
}

static void init_wavetable() {
    for (int i = 0; i < AUDIO_WAVETABLE_SIZE; i++) {
        wavetable[i] =
            (16383 * sin(2 * M_PI * i / AUDIO_WAVETABLE_SIZE)) + 16384;
    }
}

static void set_freq(int chan, float f) {
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

static void audio_pwm_handler() {
    pwm_clear_irq(AUDIO_SLICE);

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

    pwm_set_chan_level(AUDIO_SLICE, AUDIO_CHANNEL, samp);
}





