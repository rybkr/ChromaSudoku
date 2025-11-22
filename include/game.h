#ifndef GAME_H_CB9C1A5B6910FB2F
#define GAME_H_CB9C1A5B6910FB2F

#include "sudoku.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DIFFICULTY_EASY = 1,
    DIFFICULTY_MEDIUM = 2,
    DIFFICULTY_HARD = 3
} difficulty_t;

typedef struct {
    sudoku_puzzle_t puzzle;
    uint8_t cursor_row;
    uint8_t cursor_col;
    uint8_t selected_color;
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

void game_init();
void game_new_puzzle(difficulty_t difficulty);
void game_update();

void game_handle_keypad();
bool game_check_solved();
void game_draw_board();

#endif // GAME_H_CB9C1A5B6910FB2F
