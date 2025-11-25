#include "display2.h"
#include "eeprom.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

// define gpio pins connected to the LCD #2
const int SPI1_DISP_SCK = 10;
const int SPI1_DISP_CSn = 9;
const int SPI1_DISP_TX = 11;

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

// sets pins to spi role and initializes at 10khz
static void init_chardisp_pins(void) {
    gpio_set_function(SPI1_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_DISP_TX, GPIO_FUNC_SPI);
    spi_init(spi1, 10000); // once it works move to 200khz - 1MHz
    spi_set_format(spi1, 9, 0, 0, SPI_MSB_FIRST); // 9 bits, mode 0, MSB first
}

static void cd_init(void) {
    sleep_ms(15);

    send_spi_cmd(spi1, 0x028); // function set
    sleep_us(40);

    send_spi_cmd(spi1, 0x00C); // display control
    sleep_us(40);

    send_spi_cmd(spi1, 0x001); // clear display
    sleep_ms(2);

    send_spi_cmd(spi1, 0x002); // return home
    sleep_us(40);

    send_spi_cmd(spi1, 0x006); // Entry mode set
}

// sets cursor to line 1
static void cd_display1(const char *str) {
    send_spi_cmd(spi1, 0x80);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi1, str[i]);
    }
}

// sets cursor to line 2
static void cd_display2(const char *str) {
    send_spi_cmd(spi1, 0xC0);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi1, str[i]);
    }
}

void display2_init(void) {
    init_chardisp_pins();
    cd_init();
}

void display2_clear(void) {
    send_spi_cmd(spi1, 0x01);
    sleep_ms(2);
}

void display2_print_at(uint8_t row, uint8_t col, const char *msg) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    send_spi_cmd(spi1, 0x80 | (addr + col)); // Set DDRAM address

    for (int i = 0; msg[i] != '\0' && col + i < 16; i++) {
        send_spi_data(spi1, msg[i]);
    }
}

void display2_show_splash(void) {
    // NO delay and NO clear at the end – we want this to persist
    display2_clear();
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
