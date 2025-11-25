#include "display.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

// define gpio pins connected to LCD #1
const int SPI_DISP_SCK = 18;
const int SPI_DISP_CSn = 21;
const int SPI_DISP_TX = 19;

// waits until spi is free then sends to LCD
static void send_spi_cmd(spi_inst_t *spi, uint16_t value) {
    while (spi_is_busy(spi)) {
        // Wait for SPI to be ready
    }
    spi_write16_blocking(spi, &value, 1);
}

// calls send spi cmd and adds data bit
static void send_spi_data(spi_inst_t *spi, uint16_t value) {
    send_spi_cmd(spi, value | 0x100U);
}

// sets pins to spi role and initializes
static void init_chardisp_pins(void) {
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
    spi_init(spi0, 100000); // 100 kHz for safety
    // 9 bits per frame, CPOL=1, CPHA=1, MSB first (what you had working)
    spi_set_format(spi0, 9, 1, 1, SPI_MSB_FIRST);
}

static void cd_init(void) {
    sleep_ms(15);

    send_spi_cmd(spi0, 0x028); // function set
    sleep_us(40);

    send_spi_cmd(spi0, 0x00C); // display control
    sleep_us(40);

    send_spi_cmd(spi0, 0x001); // clear display
    sleep_ms(2);

    send_spi_cmd(spi0, 0x002); // return home
    sleep_us(40);

    send_spi_cmd(spi0, 0x006); // Entry mode set
}

// sets cursor to line 1
static void cd_display1(const char *str) {
    send_spi_cmd(spi0, 0x80);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
    }
}

// sets cursor to line 2
static void cd_display2(const char *str) {
    send_spi_cmd(spi0, 0xC0);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
    }
}

void display_init(void) {
    init_chardisp_pins();
    cd_init();
}

void display_clear(void) {
    send_spi_cmd(spi0, 0x01);
    sleep_ms(2);
}

void display_print_at(uint8_t row, uint8_t col, const char *msg) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    send_spi_cmd(spi0, 0x80 | (addr + col)); // Set DDRAM address

    for (int i = 0; msg[i] != '\0' && col + i < 16; i++) {
        send_spi_data(spi0, msg[i]);
    }
}

void display_show_splash(void) {
    // NO long delay and NO clear at the end
    display_clear();
    display_print_at(0, 0, " Chroma Sudoku ");
    display_print_at(1, 0, "    Team 76    ");
}

void display_show_difficulty(const char *difficulty) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Difficulty: %-6s", difficulty);
    cd_display1(buf);
    // brief pause is fine here – called once when starting game
    sleep_ms(900);
}

void display_show_timer(uint32_t seconds) {
    char buf[17];
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buf, sizeof(buf), "Time: %02lu:%02lu    ", mins, secs);
    cd_display1(buf);
}

void display_show_status(const char *msg) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%-16s", msg);
    cd_display2(buf);
}

// NEW: show difficulty/mode on line 2
void display_show_mode(const char *mode) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Mode: %-10s", mode);
    cd_display2(buf);
}

// NEW: when solved, show final vs best
void display_show_final_and_best(uint32_t final, uint32_t best) {
    char line1[17];
    char line2[17];

    uint32_t fmin = final / 60;
    uint32_t fsec = final % 60;
    uint32_t bmin = best / 60;
    uint32_t bsec = best % 60;

    snprintf(line1, sizeof(line1), "Time %02lu:%02lu", fmin, fsec);
    snprintf(line2, sizeof(line2), "Best %02lu:%02lu", bmin, bsec);

    cd_display1(line1);
    cd_display2(line2);
}
