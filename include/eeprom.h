#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stdbool.h>

// EEPROM configuration
#define EEPROM_I2C_ADDR 0x50  // Standard I2C address for 24LC256
#define EEPROM_PAGE_SIZE 64
#define EEPROM_SIZE 32768  // 32KB EEPROM

// High score structure
typedef struct {
    uint32_t score;
    char name[16];
} high_score_t;

// EEPROM functions
void eeprom_init(void);
bool eeprom_read(uint16_t addr, uint8_t *data, uint16_t len);
bool eeprom_write(uint16_t addr, const uint8_t *data, uint16_t len);
bool eeprom_write_high_score(uint8_t index, const high_score_t *score);
bool eeprom_read_high_score(uint8_t index, high_score_t *score);
bool eeprom_is_high_score(uint32_t score);
bool eeprom_insert_high_score(const high_score_t *score);
bool eeprom_get_all_high_scores(high_score_t *scores);
bool eeprom_clear_high_scores(void);
#endif // EEPROM_H
