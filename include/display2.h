#ifndef DISPLAY_H
#define DISPLAY_H

// SPI display pin configuration - update these for your hardware
extern const int SPI_DISP_SCK;
extern const int SPI_DISP_CSn;
extern const int SPI_DISP_TX;

// Display functions
void display2_init(void);
void display2_clear(void);
void display2_print_at(uint8_t row, uint8_t col, const char *msg);
void display2_show_difficulty(const char *difficulty);
void display2_show_timer(uint32_t seconds);
void display2_show_status(const char *msg);
void display2_show_splash(void);

#endif // DISPLAY_H
