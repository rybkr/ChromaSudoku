#ifndef HUB75_H_CB9C1A5B6910FB2F
#define HUB75_H_CB9C1A5B6910FB2F

#include <stdint.h>

#define HUB75_PANEL_WIDTH  32
#define HUB75_PANEL_HEIGHT 32
#define HUB75_COLOR_DEPTH  4

void hub75_init();
void hub75_refresh();

void hub75_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void hub75_clear();

void hub75_set_cursor(uint8_t x, uint8_t y);
void hub75_update(void);

#endif // HUB75_H_CB9C1A5B6910FB2F
