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

static const struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_map[9] = {
    {255,   0,   0},   // 1 - Red
    {  0, 255,   0},   // 2 - Green
    {  0,   0, 255},   // 3 - Blue
    {255, 255,   0},   // 4 - Yellow
    {255,   0, 255},   // 5 - Magenta
    {  0, 255, 255},   // 6 - Cyan
    {255,  64,   0},   // 7 - Orange
    { 64,   0, 255},   // 8 - Purple
    {255, 255, 255},   // 9 - White
};

void game_init();
void game_new_puzzle(difficulty_t difficulty);
void game_update();

void game_handle_keypad();
bool game_check_solved();
void game_draw_board();

#endif // GAME_H_CB9C1A5B6910FB2F
