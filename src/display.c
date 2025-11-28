#include "display.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

//#define SPI_DISP_SCK 27
//#define SPI_DISP_CSn 29
//#define SPI_DISP_TX 28
const int SPI_DISP_SCK = 26; // Replace with your SCK pin number for the LCD/OLED display
const int SPI_DISP_CSn = 25; // Replace with your CSn pin number for the LCD/OLED display
const int SPI_DISP_TX = 27;  // Replace with your TX pin number for the LCD/OLED display

static
void send_spi_cmd(spi_inst_t *spi, uint16_t value)
{
    while (spi_is_busy(spi))
    {
    }
    spi_write16_blocking(spi, &value, 1);
}

// calls send spi cmd and adds data bit
static
void send_spi_data(spi_inst_t *spi, uint16_t value)
{
    send_spi_cmd(spi, value | 0x200U);
}

// sets pins to spi role and initializes
static void init_chardisp_pins(void) {
    spi_init(spi1, 10000);
    spi_set_format(spi1, 10, 0, 0, SPI_MSB_FIRST);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
}

static void cd_init(void) {
    sleep_ms(1);
    send_spi_cmd(spi1, 0b0000111000);

    sleep_us(40);
    send_spi_cmd(spi1, 0b0000001100);

    sleep_us(40);
    send_spi_cmd(spi1, 0b0000000001);

    sleep_ms(2);
    send_spi_cmd(spi1, 0b0000000110);
    send_spi_cmd(spi1, 0b0000000010);

    sleep_ms(2);
}

void cd_display1(const char *str) {
    send_spi_cmd(spi1, 0x80);
    for (int i = 0; str[i] != 0; ++i)
    {
        send_spi_data(spi1, str[i]);
    }
}

void cd_display2(const char *str) {
    send_spi_cmd(spi1, 0xC0);
    for (int i = 0; str[i] != 0; ++i)
    {
        send_spi_data(spi1, str[i]);
    }
}

void display_init(void) {
    init_chardisp_pins();
    cd_init();
}

void display_clear(void) {
    send_spi_cmd(spi1, 0b0000000001);
    sleep_ms(2);
}

void display_print_at(uint8_t row, uint8_t col, const char *msg) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    send_spi_cmd(spi1, 0b0010000000 | addr | col);

    for (int i = 0; msg[i] != '\0' && col + i < 16; i++) {
        send_spi_data(spi1, msg[i]);
    }
}

void display_show_splash(void) {
    display_clear();
    display_print_at(0, 0, " Chroma Sudoku ");
    display_print_at(1, 0, "    Team 76    ");
}

void display_show_difficulty(const char *difficulty) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Difficulty: %-3s", difficulty);
    cd_display1(buf);
    // brief pause is fine here – called once when starting game
}

void display_show_timer(uint32_t seconds) {
    char buf[17];
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buf, sizeof(buf), "Time: %lu:%lu", mins, secs);
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
