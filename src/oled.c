#include "oled.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define OLED_DISPLAY1_SCK 26
#define OLED_DISPLAY1_CSn 25
#define OLED_DISPLAY1_TX  27

#define OLED_DISPLAY2_SCK 22
#define OLED_DISPLAY2_CSn 17
#define OLED_DISPLAY2_TX  23

#define OLED_COMMAND_INIT   0x038
#define OLED_COMMAND_ENABLE 0x00C
#define OLED_COMMAND_CLEAR  0x001
#define OLED_COMMAND_MODE   0x006
#define OLED_COMMAND_RETURN 0x002
#define OLED_COMMAND_DDRAM  0x080

static void send_spi_cmd(spi_inst_t *spi, uint16_t value);
static void send_spi_data(spi_inst_t *spi, uint16_t value);

void oled_init() {
    spi_init(OLED_DISPLAY1, 10000);
    spi_init(OLED_DISPLAY2, 10000);
    spi_set_format(OLED_DISPLAY1, 10, 0, 0, SPI_MSB_FIRST);
    spi_set_format(OLED_DISPLAY2, 10, 0, 0, SPI_MSB_FIRST);

    gpio_set_function(OLED_DISPLAY1_SCK, GPIO_FUNC_SPI);
    gpio_set_function(OLED_DISPLAY1_CSn, GPIO_FUNC_SPI);
    gpio_set_function(OLED_DISPLAY1_TX,  GPIO_FUNC_SPI);
    gpio_set_function(OLED_DISPLAY2_SCK, GPIO_FUNC_SPI);
    gpio_set_function(OLED_DISPLAY2_CSn, GPIO_FUNC_SPI);
    gpio_set_function(OLED_DISPLAY2_TX,  GPIO_FUNC_SPI);

    sleep_ms(1);

    send_spi_cmd(OLED_DISPLAY1, OLED_COMMAND_INIT);
    send_spi_cmd(OLED_DISPLAY2, OLED_COMMAND_INIT);
    sleep_us(40);
    
    send_spi_cmd(OLED_DISPLAY1, OLED_COMMAND_ENABLE);
    send_spi_cmd(OLED_DISPLAY2, OLED_COMMAND_ENABLE);
    sleep_us(40);

    send_spi_cmd(OLED_DISPLAY1, OLED_COMMAND_CLEAR);
    send_spi_cmd(OLED_DISPLAY2, OLED_COMMAND_CLEAR);
    sleep_ms(2);

    send_spi_cmd(OLED_DISPLAY1, OLED_COMMAND_MODE);
    send_spi_cmd(OLED_DISPLAY2, OLED_COMMAND_MODE);
    send_spi_cmd(OLED_DISPLAY1, OLED_COMMAND_RETURN);
    send_spi_cmd(OLED_DISPLAY2, OLED_COMMAND_RETURN);
    sleep_ms(2);
}

void oled_clear(spi_inst_t *oled) {
    send_spi_cmd(oled, OLED_COMMAND_CLEAR);
    sleep_ms(2);
}

void oled_display_at(spi_inst_t *oled, uint8_t row, uint8_t col, const char *msg) {
    uint8_t addr = row == 0 ? 0x00 : 0x40;
    send_spi_cmd(oled, OLED_COMMAND_DDRAM | addr | col);
    for (unsigned i = 0; msg[i] != '\0' && col + i < 16; ++i) {
        send_spi_data(oled, msg[i]);
    }
}

void oled_splash() {
    oled_clear(OLED_DISPLAY1);
    oled_display_at(OLED_DISPLAY1, 0, 0, " Chroma Sudoku ");
    oled_display_at(OLED_DISPLAY1, 1, 0, "    Team 76    ");

    oled_clear(OLED_DISPLAY2);
    // TODO: Consider displaying message on second display at startup
}

static void send_spi_cmd(spi_inst_t *spi, uint16_t value) {
    while (spi_is_busy(spi));
    spi_write16_blocking(spi, &value, 1);
}

static void send_spi_data(spi_inst_t *spi, uint16_t value) {
    send_spi_cmd(spi, value | 0x200U);
}
