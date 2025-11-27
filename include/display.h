#ifndef DISPLAY_H_CB9C1A5B6910FB2F
#define DISPLAY_H_CB9C1A5B6910FB2F

#include <stdint.h>

void display_init();
void display_clear();
void display_print_at(uint8_t row, uint8_t col, const char *msg);
void display_show_splash();

void display_show_difficulty(const char *difficulty);
void display_show_timer(uint32_t seconds);
void display_show_status(const char *msg);

void display_show_mode(const char *mode);
void display_show_final_and_best(uint32_t final, uint32_t best);

#endif // DISPLAY_H_CB9C1A5B6910FB2F
