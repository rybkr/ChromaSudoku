#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "eeprom.h"

// I2C configuration - update these for your hardware
#define I2C_EEPROM i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_BAUDRATE 100000  // 100 kHz standard speed for EEPROM

// Initialize I2C for EEPROM
void eeprom_init(void) {
    i2c_init(I2C_EEPROM, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

// Read from EEPROM
bool eeprom_read(uint16_t addr, uint8_t *data, uint16_t len) {
    uint8_t addr_bytes[2];
    addr_bytes[0] = (addr >> 8) & 0xFF;  // High byte
    addr_bytes[1] = addr & 0xFF;         // Low byte

    // Write address
    if (i2c_write_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, addr_bytes, 2, true) != 2) {
        return false;
    }

    // Read data
    return i2c_read_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, data, len, false) == len;
}

// Write to EEPROM (handles page boundaries)
bool eeprom_write(uint16_t addr, const uint8_t *data, uint16_t len) {
    uint16_t bytes_written = 0;

    while (bytes_written < len) {
        // Calculate bytes remaining in current page
        uint16_t page_offset = addr % EEPROM_PAGE_SIZE;
        uint16_t bytes_to_write = EEPROM_PAGE_SIZE - page_offset;
        if (bytes_to_write > (len - bytes_written)) {
            bytes_to_write = len - bytes_written;
        }

        // Prepare write buffer: address + data
        uint8_t write_buf[EEPROM_PAGE_SIZE + 2];
        write_buf[0] = (addr >> 8) & 0xFF;
        write_buf[1] = addr & 0xFF;
        for (uint16_t i = 0; i < bytes_to_write; i++) {
            write_buf[2 + i] = data[bytes_written + i];
        }

        // Write to EEPROM
        if (i2c_write_blocking(I2C_EEPROM, EEPROM_I2C_ADDR, write_buf, 2 + bytes_to_write, false) 
            != (2 + bytes_to_write)) {
            return false;
        }

        // Wait for write to complete (EEPROM needs time to write)
        sleep_ms(5);

        bytes_written += bytes_to_write;
        addr += bytes_to_write;
    }

    return true;
}

// Write high score at index (0-9)
bool eeprom_write_high_score(uint8_t index, const high_score_t *score) {
    if (index >= 10) {
        return false;
    }

    uint16_t addr = index * sizeof(high_score_t);
    return eeprom_write(addr, (const uint8_t *)score, sizeof(high_score_t));
}

// Read high score at index (0-9)
bool eeprom_read_high_score(uint8_t index, high_score_t *score) {
    if (index >= 10) {
        return false;
    }

    uint16_t addr = index * sizeof(high_score_t);
    return eeprom_read(addr, (uint8_t *)score, sizeof(high_score_t));
}
