#include "display2.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

// define gpio pins connected to the LCD #2
const int SPI0_DISP_SCK = 22;
const int SPI0_DISP_CSn = 17;
const int SPI0_DISP_TX = 23;

// waits until spi is free then sends to LCD
static void send_spi_cmd(spi_inst_t *spi, uint16_t value) {
    while (spi_is_busy(spi)) {
    }
    spi_write16_blocking(spi, &value, 1);
}

// calls send spi cmd and adds data bit
static void send_spi_data(spi_inst_t *spi, uint16_t value) {
    send_spi_cmd(spi, value | 0x200U);
}

// sets pins to spi role and initializes at 10khz
static void init_chardisp_pins(void) {
    spi_init(spi0, 10000);
    spi_set_format(spi0, 10, 0, 0, SPI_MSB_FIRST);
    gpio_set_function(SPI0_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_DISP_TX, GPIO_FUNC_SPI);
}

static void cd_init(void) {
    sleep_ms(1);
    send_spi_cmd(spi0, 0b0000111000);

    sleep_us(40);
    send_spi_cmd(spi0, 0b0000001100);

    sleep_us(40);
    send_spi_cmd(spi0, 0b0000000001);

    sleep_ms(2);
    send_spi_cmd(spi0, 0b0000000110);
    send_spi_cmd(spi0, 0b0000000010);

    sleep_ms(2);
}

// sets cursor to line 1
static void cd_display1(const char *str) {
    send_spi_cmd(spi0, 0x80);
    sleep_us(40);  // Wait for command to complete
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
        sleep_us(40);  // Small delay between characters
    }
}

// sets cursor to line 2
static void cd_display2(const char *str) {
    send_spi_cmd(spi0, 0xC0);
    sleep_us(40);  // Wait for command to complete
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
        sleep_us(40);  // Small delay between characters
    }
}

void display2_init(void) {
    init_chardisp_pins();
    cd_init();
}

void display2_clear(void) {
    send_spi_cmd(spi0, 0x01);
    sleep_ms(2);
}

void display2_print_at(uint8_t row, uint8_t col, const char *msg) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    send_spi_cmd(spi0, 0b0010000000 | addr | col);
    sleep_us(40);  // Wait for command to complete

    for (int i = 0; msg[i] != '\0' && col + i < 16; i++) {
        send_spi_data(spi0, msg[i]);
        sleep_us(40);  // Small delay between characters
    }
}

void display2_show_splash(void) {
    display2_clear();
    send_spi_cmd(spi0, 0b0000000010);
    display2_print_at(0, 0, " Chroma Sudoku ");
    display2_print_at(1, 0, "    Team 76    ");
}

// ---- older helpers kept for compatibility ----

void display2_show_difficulty(const char *difficulty) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Difficulty: %-6s", difficulty);
    cd_display1(buf);
}

void display2_show_timer(uint32_t seconds) {
    char buf[17];
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buf, sizeof(buf), "Time: %02lu:%02lu       ", mins, secs);
    cd_display1(buf);
}

void display2_show_status(const char *msg) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%-16s", msg);
    cd_display2(buf);
}

void display2_high_score(uint32_t seconds) {
    char buf[17];
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buf, sizeof(buf), "Best: %02lu:%02lu       ", mins, secs);
    cd_display2(buf);
}

void display2_show_instructions(void) {
    display2_clear();
    display2_print_at(0, 0, "1-9: Place Color");
    display2_print_at(1, 0, "A: New  B: Reset");
}

// ---- NEW: HUD for gameplay ----

void display2_show_game_hud(uint32_t best_seconds) {
    char line1[17];
    uint32_t mins = best_seconds / 60;
    uint32_t secs = best_seconds % 60;

    // If no best yet (0), still show 00:00
    snprintf(line1, sizeof(line1), "Best: %02lu:%02lu", mins, secs);

    display2_clear();
    cd_display1(line1);
    cd_display2("* = Help Screen");  // exactly 15 chars
}

// ---- NEW: solved screen ----

void display2_show_solved(bool new_record) {
    display2_clear();
    cd_display1("   SOLVED!    ");
    if (new_record) {
        cd_display2("NEW HIGH SCORE");
    } else {
        cd_display2("  Play Again? ");
    }
}
