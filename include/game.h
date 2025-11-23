#ifndef GAME_H_CB9C1A5B6910FB2F
#define GAME_H_CB9C1A5B6910FB2F

#include "hub75.h"
#include "sudoku.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DIFFICULTY_BEGIN = 1,
    DIFFICULTY_EASY = DIFFICULTY_BEGIN,
    DIFFICULTY_MEDIUM = 2,
    DIFFICULTY_HARD = 3,
    DIFFICULTY_COUNT,
} difficulty_t;

typedef enum {
    GAME_STATE_INTRO,
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
} game_screen_state_t;

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
    {COLOR_RED}, {COLOR_GREEN}, {COLOR_BLUE},
    {COLOR_YELLOW}, {COLOR_MAGENTA}, {COLOR_CYAN},
    {COLOR_ORANGE}, {COLOR_PURPLE}, {COLOR_WHITE},
};

void game_init();
void game_new_puzzle(difficulty_t difficulty);
void game_update();

void game_handle_keypad();
bool game_check_solved();
void game_draw_board();

#endif // GAME_H_CB9C1A5B6910FB2F
