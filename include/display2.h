#ifndef DISPLAY2_H
#define DISPLAY2_H

#include <stdint.h>
#include <stdbool.h>

// SPI display 2 pin configuration - update these for your hardware
extern const int SPI0_DISP_SCK;
extern const int SPI0_DISP_CSn;
extern const int SPI0_DISP_TX;

// Basic display-2 functions
void display2_init(void);
void display2_clear(void);
void display2_print_at(uint8_t row, uint8_t col, const char *msg);
void display2_show_splash(void);

// Old helpers (kept in case someone uses them)
void display2_show_difficulty(const char *difficulty);
void display2_show_timer(uint32_t seconds);
void display2_show_status(const char *msg);
void display2_high_score(uint32_t seconds);
void display2_show_instructions(void);

// NEW helpers for your design
void display2_show_game_hud(uint32_t best_seconds); // Best + help hint
void display2_show_solved(bool new_record);         // SOLVED / NEW HIGH SCORE

#endif // DISPLAY2_H
