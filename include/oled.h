#ifndef OLED_H_457A9C72A392D90D
#define OLED_H_457A9C72A392D90D

#include "hardware/spi.h"
#include <stdint.h>

#define OLED_DISPLAY1 spi1
#define OLED_DISPLAY2 spi0

void oled_init();

void oled_clear(spi_inst_t *oled);
void oled_display_at(spi_inst_t *oled, uint8_t row, uint8_t col, const char *msg);
void oled_splash();

#endif // OLED_H_457A9C72A392D90D
