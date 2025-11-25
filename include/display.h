#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// SPI display 1 pin configuration
extern const int SPI_DISP_SCK;
extern const int SPI_DISP_CSn;
extern const int SPI_DISP_TX;

// Basic display-1 functions
void display_init(void);
void display_clear(void);
void display_print_at(uint8_t row, uint8_t col, const char *msg);
void display_show_splash(void);

// Existing helpers
void display_show_difficulty(const char *difficulty);
void display_show_timer(uint32_t seconds);
void display_show_status(const char *msg);

// New helpers for your design
void display_show_mode(const char *mode);                         // line 2 "Mode: ..."
void display_show_final_and_best(uint32_t final, uint32_t best);  // Time vs Best

#endif // DISPLAY_H
