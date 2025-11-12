#ifndef HUB75_H
#define HUB75_H

#include <stdint.h>
#include <stdbool.h>

// HUB75 display configuration
#define HUB75_WIDTH 32
#define HUB75_HEIGHT 32
#define HUB75_NUM_PIXELS (HUB75_WIDTH * HUB75_HEIGHT)

// Color structure (RGB565 or RGB888)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

// HUB75 functions
void hub75_init(void);
void hub75_set_pixel(uint8_t x, uint8_t y, color_t color);
void hub75_clear(void);
void hub75_update(void);  // Send frame to display
void hub75_draw_sudoku_cell(uint8_t row, uint8_t col, color_t color, bool selected);

#endif // HUB75_H
