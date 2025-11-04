#ifndef DISPLAY_H
#define DISPLAY_H

// SPI display pin configuration - update these for your hardware
extern const int SPI_DISP_SCK;
extern const int SPI_DISP_CSn;
extern const int SPI_DISP_TX;

// Display functions
void display_init(void);
void display_show_difficulty(const char *difficulty);
void display_show_timer(uint32_t seconds);

#endif // DISPLAY_H
