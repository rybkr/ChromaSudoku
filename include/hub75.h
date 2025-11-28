#ifndef HUB75_H_CB9C1A5B6910FB2F
#define HUB75_H_CB9C1A5B6910FB2F

#include <stdint.h>

#define HUB75_PANEL_WIDTH  32
#define HUB75_PANEL_HEIGHT 32
#define HUB75_COLOR_DEPTH  8

#define COLOR_RED 255, 0, 0
#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_MAGENTA 255, 0, 255
#define COLOR_CYAN 0, 255, 255
#define COLOR_ORANGE 255, 64, 0
#define COLOR_PURPLE  64, 0, 255
#define COLOR_WHITE 255, 255, 255

void hub75_init();
void hub75_refresh();
void hub75_spin();

void lock_refresh();
void unlock_refresh();
void hub75_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void hub75_clear();

void hub75_set_cursor(uint8_t x, uint8_t y);
void hub75_update(void);

#endif // HUB75_H_CB9C1A5B6910FB2F
