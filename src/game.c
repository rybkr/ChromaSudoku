#include "game.h"
#include "audio.h"
#include "display.h"
#include "eeprom.h"
#include "hub75.h"
#include "keypad.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    uint8_t r, g, b;
} color_t;

static color_t number_to_color(uint8_t num);
static void draw_sudoku_cell(uint8_t row, uint8_t col, color_t color);
static void draw_cursor_ring(float row, float col);
static void get_cell_position(uint8_t row, uint8_t col, uint8_t *x, uint8_t *y);

static game_state_t game_state;

static float cursor_x = 4.0f;
static float cursor_y = 4.0f;
static const float lerp_speed = 0.2f;
static const float snap_threshold = 0.05f;
static uint32_t blink_start_time = 0;
static bool cursor_moving = false;

void game_init(void) {
    memset(&game_state, 0, sizeof(game_state_t));
    game_state.cursor_row = 4;
    game_state.cursor_col = 4;
    cursor_x = 4.0f;
    cursor_y = 4.0f;
    game_state.selected_color = 0;
    game_state.difficulty = DIFFICULTY_EASY;
    game_state.solved = false;
    blink_start_time = time_us_32() / 1000000;
}

void game_new_puzzle(difficulty_t difficulty) {
    game_state.difficulty = difficulty;
    game_state.cursor_row = game_state.cursor_col = 4;
    cursor_x = cursor_y = 4.0f;
    game_state.selected_color = 0;
    game_state.solved = false;
    blink_start_time = time_us_32() / 1000000;

    int cells_to_remove;
    switch (difficulty) {
    case DIFFICULTY_EASY:
        cells_to_remove = 36;
        break;
    case DIFFICULTY_MEDIUM:
        cells_to_remove = 42;
        break;
    case DIFFICULTY_HARD:
        cells_to_remove = 50;
        break;
    default:
        cells_to_remove = 36;
    }

    clear(&game_state.puzzle);
    solve_puzzle(&game_state.puzzle);
    create_puzzle_from_solution(&game_state.puzzle, cells_to_remove);

    game_state.start_time = time_us_32() / 1000000;
    game_state.elapsed_time = 0;

    const char *diff_str;
    switch (difficulty) {
    case DIFFICULTY_EASY:
        diff_str = "Easy";
        break;
    case DIFFICULTY_MEDIUM:
        diff_str = "Med";
        break;
    case DIFFICULTY_HARD:
        diff_str = "Hard";
        break;
    default:
        diff_str = "Easy";
    }
    display_show_difficulty(diff_str);
}

void game_update(void) {
    uint32_t current_time = time_us_32() / 1000000;
    game_state.elapsed_time = current_time - game_state.start_time;

    display_show_timer(game_state.elapsed_time);

    game_handle_keypad();

    float dx = game_state.cursor_col - cursor_x;
    float dy = game_state.cursor_row - cursor_y;

    if (fabsf(dx) > snap_threshold || fabsf(dy) > snap_threshold) {
        cursor_x += dx * lerp_speed;
        cursor_y += dy * lerp_speed;
        cursor_moving = true;
        blink_start_time = current_time;
    } else {
        cursor_x = game_state.cursor_col;
        cursor_y = game_state.cursor_row;
        cursor_moving = false;
    }

    if (!game_state.solved && game_check_solved()) {
        game_state.solved = true;
        audio_play_victory_tune();
    }

    game_draw_board();
}

void game_handle_keypad(void) {
    while (1) {
        uint16_t event = keypad_get_event();
        if (event == 0) {
            break;
        }

        if (keypad_is_pressed(event)) {
            char key = keypad_get_char(event);

            switch (key) {
            case '#':
                if (game_state.cursor_col > 0) {
                    game_state.cursor_col--;
                }
                break;
            case '*':
                if (game_state.cursor_row < 8) {
                    game_state.cursor_row++;
                }
                break;
            case 'D':
                if (game_state.cursor_row > 0) {
                    game_state.cursor_row--;
                }
                break;
            case '0':
                if (game_state.cursor_col < 8) {
                    game_state.cursor_col++;
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                game_state.selected_color = key - '1';
                set(&game_state.puzzle, game_state.cursor_row,
                    game_state.cursor_col, game_state.selected_color + 1);
                break;
            default:
                break;
            }
        }
    }
}

bool game_check_solved(void) {
    return is_valid(&game_state.puzzle);
}

void game_draw_board(void) {
    hub75_clear();

    for (uint8_t row = 0; row < 9; row++) {
        for (uint8_t col = 0; col < 9; col++) {
            uint8_t value = get(&game_state.puzzle, row, col);
            color_t color = number_to_color(value);
            draw_sudoku_cell(row, col, color);
        }
    }

    uint32_t current_time = time_us_32() / 1000000;
    uint32_t time_since_move = current_time - blink_start_time;
    bool show_cursor = cursor_moving || ((time_since_move % 2) == 0);

    if (show_cursor) {
        draw_cursor_ring(cursor_y, cursor_x);
    }

    hub75_refresh();
}

static color_t number_to_color(uint8_t num) {
    if (num == 0 || num > 9) {
        color_t black = {0, 0, 0};
        return black;
    }
    color_t color;
    color.r = color_map[num - 1].r;
    color.g = color_map[num - 1].g;
    color.b = color_map[num - 1].b;
    return color;
}

static void get_cell_position(uint8_t row, uint8_t col, uint8_t *x, uint8_t *y) {
    *x = 2 + (col / 3) + (col * 3);
    *y = 2 + (row / 3) + (row * 3);
}

static void draw_sudoku_cell(uint8_t row, uint8_t col, color_t color) {
    uint8_t start_x, start_y;
    get_cell_position(row, col, &start_x, &start_y);

    for (uint8_t dy = 0; dy < 2; ++dy) {
        for (uint8_t dx = 0; dx < 2; ++dx) {
            hub75_set_pixel(start_x + dx, start_y + dy, color.r, color.g, color.b);
        }
    }
}

static void draw_cursor_ring(float row, float col) {
    float px = 2.0f + ((int)col / 3) + (col * 3.0f);
    float py = 2.0f + ((int)row / 3) + (row * 3.0f);

    int16_t start_x = (int16_t)(px + 0.5f);
    int16_t start_y = (int16_t)(py + 0.5f);

    for (int16_t x = start_x - 1; x <= start_x + 2; x++) {
        if (x >= 0 && x < HUB75_PANEL_WIDTH && start_y - 1 >= 0) {
            hub75_set_pixel(x, start_y - 1, 255, 255, 255);
        }
    }
    for (int16_t x = start_x - 1; x <= start_x + 2; x++) {
        if (x >= 0 && x < HUB75_PANEL_WIDTH && start_y + 2 < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(x, start_y + 2, 255, 255, 255);
        }
    }
    for (int16_t y = start_y; y <= start_y + 1; y++) {
        if (start_x - 1 >= 0 && y >= 0 && y < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(start_x - 1, y, 255, 255, 255);
        }
    }
    for (int16_t y = start_y; y <= start_y + 1; y++) {
        if (start_x + 2 < HUB75_PANEL_WIDTH && y >= 0 && y < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(start_x + 2, y, 255, 255, 255);
        }
    }
}
