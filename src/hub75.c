#include "hub75.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdint.h>

#define HUB75_R0_PIN 9
#define HUB75_G0_PIN 10
#define HUB75_B0_PIN 8
#define HUB75_R1_PIN 7
#define HUB75_G1_PIN 12
#define HUB75_B1_PIN 6
#define HUB75_CLK_PIN 19
#define HUB75_LAT_PIN 16
#define HUB75_OE_PIN 18
#define HUB75_A_PIN 21
#define HUB75_B_PIN 14
#define HUB75_C_PIN 20
#define HUB75_D_PIN 15

static color_t frame_buffer[HUB75_NUM_PIXELS];

void hub75_init(void) {
    gpio_init(HUB75_LAT_PIN);
    gpio_set_dir(HUB75_LAT_PIN, GPIO_OUT);
    gpio_put(HUB75_LAT_PIN, 0);

    gpio_init(HUB75_OE_PIN);
    gpio_set_dir(HUB75_OE_PIN, GPIO_OUT);
    gpio_put(HUB75_OE_PIN, 1);

    gpio_init(HUB75_A_PIN);
    gpio_set_dir(HUB75_A_PIN, GPIO_OUT);
    gpio_init(HUB75_B_PIN);
    gpio_set_dir(HUB75_B_PIN, GPIO_OUT);
    gpio_init(HUB75_C_PIN);
    gpio_set_dir(HUB75_C_PIN, GPIO_OUT);
    gpio_init(HUB75_D_PIN);
    gpio_set_dir(HUB75_D_PIN, GPIO_OUT);

    gpio_init(HUB75_R0_PIN);
    gpio_set_dir(HUB75_R0_PIN, GPIO_OUT);
    gpio_init(HUB75_G0_PIN);
    gpio_set_dir(HUB75_G0_PIN, GPIO_OUT);
    gpio_init(HUB75_B0_PIN);
    gpio_set_dir(HUB75_B0_PIN, GPIO_OUT);
    gpio_init(HUB75_R1_PIN);
    gpio_set_dir(HUB75_R1_PIN, GPIO_OUT);
    gpio_init(HUB75_G1_PIN);
    gpio_set_dir(HUB75_G1_PIN, GPIO_OUT);
    gpio_init(HUB75_B1_PIN);
    gpio_set_dir(HUB75_B1_PIN, GPIO_OUT);
    gpio_init(HUB75_CLK_PIN);
    gpio_set_dir(HUB75_CLK_PIN, GPIO_OUT);

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
#define BIT_DEPTH 4
#define NUM_ROWS (HUB75_HEIGHT / 2) // 16 rows for 32x32 panel

    for (uint8_t bit_plane = 0; bit_plane < BIT_DEPTH; bit_plane++) {
        uint8_t bit_mask = 1 << bit_plane;

        for (uint8_t row = 0; row < NUM_ROWS; row++) {
            gpio_put(HUB75_OE_PIN, 1);

            gpio_put(HUB75_A_PIN, (row >> 0) & 1);
            gpio_put(HUB75_B_PIN, (row >> 1) & 1);
            gpio_put(HUB75_C_PIN, (row >> 2) & 1);
            gpio_put(HUB75_D_PIN, (row >> 3) & 1);

            for (uint8_t col = 0; col < HUB75_WIDTH; col++) {
                color_t top_pixel = frame_buffer[row * HUB75_WIDTH + col];
                color_t bottom_pixel =
                    frame_buffer[(row + NUM_ROWS) * HUB75_WIDTH + col];

                uint8_t r0 = (top_pixel.r & bit_mask) ? 1 : 0;
                uint8_t g0 = (top_pixel.g & bit_mask) ? 1 : 0;
                uint8_t b0 = (top_pixel.b & bit_mask) ? 1 : 0;
                uint8_t r1 = (bottom_pixel.r & bit_mask) ? 1 : 0;
                uint8_t g1 = (bottom_pixel.g & bit_mask) ? 1 : 0;
                uint8_t b1 = (bottom_pixel.b & bit_mask) ? 1 : 0;

                gpio_put(HUB75_R0_PIN, r0);
                gpio_put(HUB75_G0_PIN, g0);
                gpio_put(HUB75_B0_PIN, b0);
                gpio_put(HUB75_R1_PIN, r1);
                gpio_put(HUB75_G1_PIN, g1);
                gpio_put(HUB75_B1_PIN, b1);

                gpio_put(HUB75_CLK_PIN, 1);
                sleep_us(1); // Short delay
                gpio_put(HUB75_CLK_PIN, 0);
                sleep_us(1); // Short delay
            }

            gpio_put(HUB75_LAT_PIN, 1);
            sleep_us(1);
            gpio_put(HUB75_LAT_PIN, 0);
            sleep_us(1);

            gpio_put(HUB75_OE_PIN, 0);

            busy_wait_us(10 << bit_plane); // 10, 20, 40, 80 microseconds
            gpio_put(HUB75_OE_PIN, 1);
        }
    }
}

void hub75_draw_sudoku_cell(uint8_t row, uint8_t col, color_t color,
                            bool selected) {
    // Each Sudoku cell is 2x2 LEDs
    // Position calculation: 2 pixels per cell + 1 pixel gap + 2 pixel border
    // Total: 2*9 + 1*6 + 2*2 = 18 + 8 + 4 = 28, leaving 2 pixels for margin

    uint8_t start_x = 2 + (col / 3) + (col * 3);
    uint8_t start_y = 2 + (row / 3) + (row * 3);

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

void hub75_draw_sudoku_cell_with_ring(uint8_t row, uint8_t col, color_t color,
                                      bool selected, bool show_ring) {
    // Draw the cell first
    hub75_draw_sudoku_cell(row, col, color, selected);

    // Draw white ring around selected cell if show_ring is true
    if (selected && show_ring) {
        uint8_t start_x = 2 + (col / 3) + (col * 3);
        uint8_t start_y = 2 + (row / 3) + (row * 3);

        color_t white = {255, 255, 255};

        // Draw ring around the 2x2 cell (from start_x-1 to start_x+2, start_y-1
        // to start_y+2) Top border (4 pixels wide)
        if (start_y > 0) {
            for (int8_t x = start_x - 1; x <= start_x + 2; x++) {
                if (x >= 0 && x < HUB75_WIDTH) {
                    hub75_set_pixel(x, start_y - 1, white);
                }
            }
        }
        // Bottom border (4 pixels wide)
        if (start_y + 2 < HUB75_HEIGHT) {
            for (int8_t x = start_x - 1; x <= start_x + 2; x++) {
                if (x >= 0 && x < HUB75_WIDTH) {
                    hub75_set_pixel(x, start_y + 2, white);
                }
            }
        }
        // Left border (2 pixels tall, excluding corners already drawn)
        if (start_x > 0) {
            for (uint8_t y = start_y; y <= start_y + 1; y++) {
                if (y < HUB75_HEIGHT) {
                    hub75_set_pixel(start_x - 1, y, white);
                }
            }
        }
        // Right border (2 pixels tall, excluding corners already drawn)
        if (start_x + 2 < HUB75_WIDTH) {
            for (uint8_t y = start_y; y <= start_y + 1; y++) {
                if (y < HUB75_HEIGHT) {
                    hub75_set_pixel(start_x + 2, y, white);
                }
            }
        }
    }
}
