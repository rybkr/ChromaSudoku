#ifndef GAME_H_48146DFD52ED1E93
#define GAME_H_48146DFD52ED1E93

#include "hub75.h"
#include "sudoku.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DIFFICULTY_BEGIN = 0,
    DIFFICULTY_EASY = DIFFICULTY_BEGIN,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
    DIFFICULTY_COUNT,
    DIFFICULTY_DEFAULT = DIFFICULTY_EASY,
} difficulty_t;

typedef enum {
    GAME_STATE_INTRO,
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
} game_screen_state_t;

static const char *DIFFICULTY_NAMES[] = {
    [DIFFICULTY_EASY] = "Easy",
    [DIFFICULTY_MEDIUM] = "Med",
    [DIFFICULTY_HARD] = "Hard",
};

typedef struct {
    sudoku_puzzle_t puzzle;
    difficulty_t difficulty;
    bool solved;

    uint8_t cursor_row;
    uint8_t cursor_col;
    uint8_t selected_color;

    uint32_t start_time;
    uint32_t elapsed_time;
    uint32_t best_time;
    
} game_state_t;

typedef struct {
    uint8_t r, g, b;
} color_t;

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
void game_handle_joystick();
bool game_check_solved();
void game_draw_board();

#endif // GAME_H_48146DFD52ED1E93
