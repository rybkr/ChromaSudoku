#include "hub75.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdint.h>

#define HUB75_R0_PIN 0
#define HUB75_G0_PIN 1
#define HUB75_B0_PIN 2
#define HUB75_R1_PIN 3
#define HUB75_G1_PIN 4
#define HUB75_B1_PIN 5
#define HUB75_CLK_PIN 6
#define HUB75_LAT_PIN 7
#define HUB75_OE_PIN 8
#define HUB75_A_PIN 9
#define HUB75_B_PIN 10
#define HUB75_C_PIN 11
#define HUB75_D_PIN 12

static color_t frame_buffer[HUB75_NUM_PIXELS];

void hub75_init(void) {
    // TODO(rybkr): Initialize PIO state machine for HUB75 protocol
    hub75_clear();
}

void hub75_set_pixel(uint8_t x, uint8_t y, color_t color) {
    if (x >= HUB75_WIDTH || y >= HUB75_HEIGHT) {
        return;
    }
    frame_buffer[y * HUB75_WIDTH + x] = color;
}

void hub75_clear(void) {
    color_t black = {0, 0, 0};
    for (int i = 0; i < HUB75_NUM_PIXELS; i++) {
        frame_buffer[i] = black;
    }
}

void hub75_update(void) {
    // TODO(rybkr): Send frame_buffer to display via PIO
    // This will use DMA to transfer data efficiently
}

void hub75_draw_sudoku_cell(uint8_t row, uint8_t col, color_t color,
                                                        bool selected) {
    // Each Sudoku cell is 2x2 LEDs
    // Position calculation: 2 pixels per cell + 1 pixel gap + 2 pixel border
    // Total: 2*9 + 1*8 + 2*2 = 18 + 8 + 4 = 30, leaving 2 pixels for margin

    uint8_t start_x = col * 3 + 1;
    uint8_t start_y = row * 3 + 1;

    color_t display_color = color;
    if (selected) {
        display_color.r = (display_color.r + 255) / 2;
        display_color.g = (display_color.g + 255) / 2;
        display_color.b = (display_color.b + 255) / 2;
    }

    for (uint8_t dy = 0; dy < 2; dy++) {
        for (uint8_t dx = 0; dx < 2; dx++) {
            hub75_set_pixel(start_x + dx, start_y + dy, display_color);
        }
    }
}
