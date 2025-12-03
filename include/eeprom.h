#ifndef EEPROM_H_57E95025B865A3A1
#define EEPROM_H_57E95025B865A3A1

#include "game.h"
#include <stdint.h>
#include <stdbool.h>

// High scores will be represented as number of seconds to solve.
typedef uint32_t high_score_t;

void eeprom_init();

bool eeprom_read(uint16_t addr, uint8_t *data, uint16_t len);
bool eeprom_read_high_score(difficulty_t difficulty, high_score_t *score);
bool eeprom_is_high_score(difficulty_t difficulty, uint32_t score);

bool eeprom_write(uint16_t addr, const uint8_t *data, uint16_t len);
bool eeprom_write_high_score(difficulty_t difficulty, const high_score_t *score);
bool eeprom_clear_high_scores();

#endif // EEPROM_H_57E95025B865A3A1
