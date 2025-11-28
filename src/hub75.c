#include "hub75.h"
#include "hub75.pio.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#define HUB75_R1_PIN 6U
#define HUB75_G1_PIN 7U
#define HUB75_B1_PIN 8U
#define HUB75_R2_PIN 9U
#define HUB75_G2_PIN 10U
#define HUB75_B2_PIN 11U

#define HUB75_A_PIN 14U
#define HUB75_B_PIN 15U
#define HUB75_C_PIN 16U
#define HUB75_D_PIN 18U

#define HUB75_CLK_PIN 12U
#define HUB75_LAT_PIN 19U
#define HUB75_OE_PIN 20U

static PIO pio = pio0;
static unsigned sm;
static uint8_t framebuffer[HUB75_PANEL_HEIGHT][HUB75_PANEL_WIDTH][3];

static float cursor_x = 0;
static float cursor_y = 0;
static float target_x = 0;
static float target_y = 0;

static const float lerp_speed = .3f;

static volatile bool is_reading = false;
static volatile uint32_t last_write = 0;
static volatile bool refresh_lock = false;

static void set_row_address(uint8_t row) {
    gpio_put(HUB75_A_PIN, row & 0x1U);
    gpio_put(HUB75_B_PIN, row & 0x2U);
    gpio_put(HUB75_C_PIN, row & 0x4U);
    gpio_put(HUB75_D_PIN, row & 0x8U);
}

static void refresh_row(uint8_t row, uint8_t bit) {
    uint8_t row_top = row;
    uint8_t row_bot = row + (HUB75_PANEL_HEIGHT / 2);
    uint8_t mask = 1U << (8 - HUB75_COLOR_DEPTH + bit);

    gpio_put(HUB75_OE_PIN, 1);

    for (int x = 0; x < HUB75_PANEL_WIDTH; ++x) {
        unsigned pixel =
            ((framebuffer[row_top][x][0] & mask) ? (1U << 0) : 0) |
            ((framebuffer[row_top][x][1] & mask) ? (1U << 1) : 0) |
            ((framebuffer[row_top][x][2] & mask) ? (1U << 2) : 0) |
            ((framebuffer[row_bot][x][0] & mask) ? (1U << 3) : 0) |
            ((framebuffer[row_bot][x][1] & mask) ? (1U << 4) : 0) |
            ((framebuffer[row_bot][x][2] & mask) ? (1U << 5) : 0);
        pio->txf[sm] = pixel; 
    }

    while (!pio_sm_is_tx_fifo_empty(pio, sm)) {
        tight_loop_contents();
    }

    set_row_address(row);
    gpio_put(HUB75_LAT_PIN, 1);
    gpio_put(HUB75_LAT_PIN, 0);

    gpio_put(HUB75_OE_PIN, 0);
    sleep_us(4U << bit);
    gpio_put(HUB75_OE_PIN, 1);
}

void hub75_refresh(void) {
    is_reading = true;
    for (uint8_t bit = 0; bit < HUB75_COLOR_DEPTH; ++bit) {
        for (uint8_t row = 0; row <  HUB75_PANEL_HEIGHT / 2; ++row) {
            refresh_row(row, bit);
        }
    }
    is_reading = false;
}

void hub75_spin() {
    while (1) {
        if (!refresh_lock && (time_us_32() - last_write) >= 1000) {
            hub75_refresh();
            sleep_ms(1);
        }
    }
}

void lock_refresh() {
    refresh_lock = true;
}

void unlock_refresh() {
    refresh_lock = false;
}

void hub75_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    while (is_reading) {
        tight_loop_contents();
    }
    if (x < HUB75_PANEL_WIDTH && y < HUB75_PANEL_HEIGHT) {
        framebuffer[y][x][0] = r;
        framebuffer[y][x][1] = g;
        framebuffer[y][x][2] = b;
    }
    last_write = time_us_32() / 1000;
}

void hub75_clear(void) {
    while (is_reading) {
        tight_loop_contents();
    }
    for (int y = 0; y < HUB75_PANEL_HEIGHT; y++) {
        for (int x = 0; x < HUB75_PANEL_WIDTH; x++) {
            framebuffer[y][x][0] = 0;
            framebuffer[y][x][1] = 0;
            framebuffer[y][x][2] = 0;
        }
    }
    last_write = time_us_32() / 1000;
}

void hub75_init(void) {
    const uint8_t ctrl_pins[] = {
        HUB75_A_PIN, HUB75_B_PIN, HUB75_C_PIN, HUB75_D_PIN,
        HUB75_LAT_PIN,
        HUB75_OE_PIN,
    };
    for (int i = 0; i < sizeof(ctrl_pins); i++) {
        gpio_init(ctrl_pins[i]);
        gpio_set_dir(ctrl_pins[i], GPIO_OUT);
    }
    
    gpio_put(HUB75_OE_PIN, 1);
    gpio_put(HUB75_LAT_PIN, 0);

    unsigned offset = pio_add_program(pio, &hub75_program);
    sm = pio_claim_unused_sm(pio, true);
    hub75_program_init(pio, sm, offset, HUB75_R1_PIN);
    set_row_address(0);
    hub75_clear();
}

void hub75_set_cursor(uint8_t x, uint8_t y) {
    target_x = x;
    target_y = y;
}

void hub75_update(void) {
    cursor_x += (target_x - cursor_x) * lerp_speed;
    cursor_y += (target_y - cursor_y) * lerp_speed;
}

uint8_t hub75_get_cursor_x(void) {
    return (uint8_t)(cursor_x + 0.5f);
}

uint8_t hub75_get_cursor_y(void) {
    return (uint8_t)(cursor_y + 0.5f);
}
