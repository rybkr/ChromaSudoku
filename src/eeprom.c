#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "eeprom.h"
#include <string.h>

// TODO: Document EEPROM Memory Layout
// DESIGN CONSIDERATIONS:
// - High Scores: bytes 0x0000 - 0x00F3 (10 entries × 20 bytes each = 240 bytes)
//   * Each entry: uint32_t (score) + char[16] (name) = 20 bytes
//   * Index 0 = highest score, Index 9 = lowest score
// - Future expansion: Game state, settings, statistics at higher addresses
// - EEPROM is 32KB (0x8000), so plenty of space for future features
// - Consider implementing wear-leveling if frequent writes needed

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

// TODO: Implement eeprom_is_high_score()
// LOGIC:
// - Read all 10 high scores from EEPROM
// - Compare the input score with each entry's score
// - Return true if: (score > any existing score) OR (less than 10 scores stored)
// - Need to handle uninitialized EEPROM (all 0xFF bytes)
// - Check if there's an empty slot (score == 0 and name[0] == '\0' or 0xFF)
// TASK: Implement the function signature and logic
bool eeprom_is_high_score(uint32_t score) {
    high_score_t entries[10];

    if (!eeprom_read(0, (uint8_t *)entries, sizeof(entries))) {
        return false;
    }

    uint8_t valid_entries = 0;
    uint32_t lowest_score = UINT32_MAX;

    for (uint8_t i = 0; i < 10; ++i) {
        const high_score_t *entry = &entries[i];
        const uint32_t entry_score = entry->score;
        const uint8_t first_char = (uint8_t)entry->name[0];

        // Skip uninitialized or empty records
        if ((entry_score == 0xFFFFFFFF && first_char == 0xFF) ||
            (entry_score == 0 && (first_char == 0 || first_char == 0xFF))) {
            continue;
        }

        ++valid_entries;
        if (entry_score < lowest_score) {
            lowest_score = entry_score;
        }

        if (score > entry_score) {
            return true;
        }
    }

    if (valid_entries < 10) {
        return true;
    }

    return score > lowest_score;
}

// TODO: Implement eeprom_insert_high_score()
// LOGIC:
// - Read all 10 high scores
// - Find the correct position where new score should go (sorted descending)
// - If top 10 is full, only add if score is better than #10
// - Shift lower scores down (lose #10 if full)
// - Write the high score struct with name at correct position
// - Save all 10 scores back to EEPROM
// TASK: Implement the function signature and logic
bool eeprom_insert_high_score(const high_score_t *score) {
    if (score == NULL) {
        return false;
    }

    high_score_t stored[10];
    if (!eeprom_read(0, (uint8_t *)stored, sizeof(stored))) {
        return false;
    }

    high_score_t compact[10];
    uint8_t count = 0;

    for (uint8_t i = 0; i < 10; ++i) {
        const high_score_t *entry = &stored[i];
        const uint32_t entry_score = entry->score;
        const uint8_t first_char = (uint8_t)entry->name[0];

        const bool invalid = (entry_score == 0xFFFFFFFF && first_char == 0xFF) ||
                             (entry_score == 0 && (first_char == 0 || first_char == 0xFF));
        if (invalid) {
            continue;
        }

        compact[count++] = *entry;
    }

    uint8_t insert_pos = 0;
    while (insert_pos < count && score->score <= compact[insert_pos].score) {
        ++insert_pos;
    }

    if (count == 10 && insert_pos == count) {
        return false;
    }

    const uint8_t new_count = count < 10 ? count + 1 : 10;
    for (int8_t idx = new_count - 1; idx > (int8_t)insert_pos; --idx) {
        compact[idx] = compact[idx - 1];
    }
    compact[insert_pos] = *score;

    high_score_t out[10];
    memset(out, 0xFF, sizeof(out));
    memcpy(out, compact, new_count * sizeof(high_score_t));

    return eeprom_write(0, (const uint8_t *)out, sizeof(out));
}

// TODO: Implement eeprom_get_all_high_scores()
// LOGIC:
// - Read all 10 high score entries from EEPROM into output buffer
// - Handle uninitialized EEPROM gracefully (scores might be 0xFF)
// - Return count of valid scores (non-zero/non-empty entries)
// - Caller will use this to display top scores
// TASK: Implement the function signature and logic
// Note: Consider if you want to return just filled slots or all 10
bool eeprom_get_all_high_scores(high_score_t *scores) {
    if (scores == NULL) {
        return false;
    }

    high_score_t raw[10];
    if (!eeprom_read(0, (uint8_t *)raw, sizeof(raw))) {
        return false;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < 10; ++i) {
        const uint32_t entry_score = raw[i].score;
        const uint8_t first_char = (uint8_t)raw[i].name[0];
        const bool empty = ((entry_score == 0 || entry_score == 0xFFFFFFFFu) &&
                            (first_char == 0x00 || first_char == 0xFF));

        if (empty) {
            continue;
        }

        scores[count++] = raw[i];
    }

    if (count < 10) {
        memset(&scores[count], 0, (10 - count) * sizeof(high_score_t));
    }

    return true;
}

// TODO: Implement eeprom_clear_high_scores()
// LOGIC:
// - Clear all 10 high score slots
// - Write zeros or 0xFF pattern to high score region
// - Used for factory reset or testing
// TASK: Implement the function signature and logic (optional but useful)
bool eeprom_clear_high_scores(void) {
    uint8_t blank[10 * sizeof(high_score_t)];
    memset(blank, 0xFF, sizeof(blank));
    return eeprom_write(0, blank, sizeof(blank));
}
