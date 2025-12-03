#include "eeprom.h"
#include "game.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <string.h>

#define EEPROM_I2C_ADDR 0x50
#define EEPROM_PAGE_SIZE 0x20
#define EEPROM_SIZE 0x8000

#define I2C_EEPROM i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_BAUDRATE 100000

#define EEPROM_MAGIC 0xCAFEBABE
#define EEPROM_MAGIC_ADDR (DIFFICULTY_COUNT * sizeof(high_score_t))

void eeprom_init() {
    i2c_init(I2C_EEPROM, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

bool eeprom_read(uint16_t addr, uint8_t *data, uint16_t len) {
    uint8_t addr_bytes[2];
    addr_bytes[0] = (addr >> 8) & 0xFF;
    addr_bytes[1] = addr & 0xFF;
    if (i2c_write_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, addr_bytes, 2, true) != 2) {
        return false;
    }
    return i2c_read_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, data, len, false) == len;
}

bool eeprom_write(uint16_t addr, const uint8_t *data, uint16_t len) {
    uint16_t bytes_written = 0;

    while (bytes_written < len) {
        uint16_t page_offset = addr % EEPROM_PAGE_SIZE;
        uint16_t bytes_to_write = EEPROM_PAGE_SIZE - page_offset;
        if (bytes_to_write > (len - bytes_written)) {
            bytes_to_write = len - bytes_written;
        }

        uint8_t write_buf[EEPROM_PAGE_SIZE + 2];
        write_buf[0] = (addr >> 8) & 0xFF;
        write_buf[1] = addr & 0xFF;
        for (uint16_t i = 0; i < bytes_to_write; i++) {
            write_buf[2 + i] = data[bytes_written + i];
        }

        if (i2c_write_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, write_buf, 2 + bytes_to_write, false) != (2 + bytes_to_write)) {
            return false;
        }

        sleep_ms(5);
        bytes_written += bytes_to_write;
        addr += bytes_to_write;
    }

    return true;
}

bool eeprom_write_high_score(difficulty_t difficulty, const high_score_t *score) {
    if (difficulty >= DIFFICULTY_COUNT) {
        return false;
    }
    uint16_t addr = (unsigned)difficulty * sizeof(high_score_t);
    return eeprom_write(addr, (const uint8_t *)score, sizeof(high_score_t));
}

bool eeprom_read_high_score(difficulty_t difficulty, high_score_t *score) {
    if (difficulty >= DIFFICULTY_COUNT) {
        return false;
    }
    uint16_t addr = (unsigned)difficulty * sizeof(high_score_t);
    return eeprom_read(addr, (uint8_t *)score, sizeof(high_score_t));
}

bool eeprom_is_high_score(difficulty_t difficulty, uint32_t score) {
    high_score_t entries[DIFFICULTY_COUNT];
    if (!eeprom_read(0, (uint8_t *)entries, sizeof(entries))) {
        return false;
    }
    return score < entries[difficulty];
}

bool eeprom_clear_high_scores() {
    const high_score_t hs = 5999;
    for (int i = DIFFICULTY_BEGIN; i < DIFFICULTY_COUNT; i++) {
        if (!eeprom_write_high_score(i, &hs)) {
            return false;
        }
    }
    return true;
}
