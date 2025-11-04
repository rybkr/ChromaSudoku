#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "display.h"

const int SPI_DISP_SCK = 18;
const int SPI_DISP_CSn = 21;
const int SPI_DISP_TX = 19;

static void send_spi_cmd(spi_inst_t *spi, uint16_t value) {
    while (spi_is_busy(spi)) {
        // Wait for SPI to be ready
    }
    spi_write16_blocking(spi, &value, 1);
}

static void send_spi_data(spi_inst_t *spi, uint16_t value) {
    send_spi_cmd(spi, value | 0x100U);
}

static void init_chardisp_pins(void) {
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
    spi_init(spi0, 10000);
    spi_set_format(spi0, 9, 0, 0, SPI_MSB_FIRST);
}

static void cd_init(void) {
    sleep_ms(1);

    send_spi_cmd(spi0, 0b0000101100);
    sleep_us(40);
    
    send_spi_cmd(spi0, 0b0000001100);
    sleep_us(40);
    
    send_spi_cmd(spi0, 1);
    sleep_ms(2);
    
    send_spi_cmd(spi0, 2);
    sleep_us(40);
}

static void cd_display1(const char *str) {
    send_spi_cmd(spi0, 2);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
    }
}

static void cd_display2(const char *str) {
    send_spi_cmd(spi0, 0x2C);
    for (int i = 0; str[i] != 0; ++i) {
        send_spi_data(spi0, str[i]);
    }
}

void display_init(void) {
    init_chardisp_pins();
    cd_init();
}

void display_show_difficulty(const char *difficulty) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Difficulty: %-6s", difficulty);
    cd_display1(buf);
}

void display_show_timer(uint32_t seconds) {
    char buf[17];
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buf, sizeof(buf), "Time: %02lu:%02lu       ", mins, secs);
    cd_display2(buf);
}
