#ifndef AUDIO_H_A6AEA1513B23504C
#define AUDIO_H_A6AEA1513B23504C

void audio_init();
void audio_update();
void audio_stop();

void audio_play_frequency(float freq);

void audio_play_game_start();
void audio_play_victory_tune();
void audio_play_blip();

#endif // AUDIO_H_A6AEA1513B23504C
