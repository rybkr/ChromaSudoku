#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// SPI display pin configuration - update these for your hardware
extern const int SPI_DISP_SCK;
extern const int SPI_DISP_CSn;
extern const int SPI_DISP_TX;

// Display functions
void display_init(void);
void display_clear(void);
void display_print_at(uint8_t row, uint8_t col, const char *msg);
void display_show_difficulty(const char *difficulty);
void display_show_timer(uint32_t seconds);
void display_show_status(const char *msg);
void display_show_splash(void);
void display_high_score(uint32_t seconds);

#endif // DISPLAY_H
