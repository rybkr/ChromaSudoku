#ifndef GAME_H
#define GAME_H

#include "sudoku.h"
#include <stdbool.h>
#include <stdint.h>

// Game difficulty levels
typedef enum {
    DIFFICULTY_EASY = 1,
    DIFFICULTY_MEDIUM = 2,
    DIFFICULTY_HARD = 3
} difficulty_t;

// Game state
typedef struct {
    sudoku_puzzle_t puzzle;
    uint8_t cursor_row;
    uint8_t cursor_col;
    uint8_t selected_color; // 0-8 for 9 colors
    difficulty_t difficulty;
    uint32_t start_time;
    uint32_t elapsed_time;
    bool solved;
} game_state_t;

// Color mapping (9 distinct colors for 9 Sudoku numbers)
static const struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_map[9] = {
    {220, 20, 60},   // Red - 1
    {24, 139, 34},   // Green - 2
    {30, 144, 255},  // Blue - 3
    {255, 215, 0},   // Yellow - 4
    {255, 0, 144},   // Magenta - 5
    {0, 206, 209},   // Cyan - 6
    {255, 140, 0},   // Orange - 7
    {138, 43, 226},  // Purple - 8
    {139, 69, 19},   // Brown - 9
};

// Game functions
void game_init(void);
void game_new_puzzle(difficulty_t difficulty);
void game_update(void); // Main game loop update
void game_handle_keypad(void);
bool game_check_solved(void);
void game_draw_board(void);

#endif // GAME_H
