#include "game.h"
#include "audio.h"
#include "display.h"
#include "display2.h"
#include "eeprom.h"
#include "hub75.h"
#include "font.h"
#include "keypad.h"
#include "joystick.h"
#include "rng.h"
#include "sudoku.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    uint8_t r, g, b;
} color_t;

typedef enum {
    GAME_STATE_INTRO,
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
} game_screen_state_t;

static void draw_sudoku_puzzle(sudoku_puzzle_t *puzzle);
static void draw_sudoku_cell(uint8_t row, uint8_t col, color_t color);
static void draw_cursor_ring(float row, float col);

static void draw_intro_screen();
static void draw_help_screen();
static void draw_color_rush_animation(uint32_t time_ms);

static void get_cell_position(uint8_t row, uint8_t col, uint8_t *x, uint8_t *y);
static color_t number_to_color(uint8_t num);

static void create_puzzle_from_solution(sudoku_puzzle_t *puzzle, int cells_to_remove);
static unsigned cells_to_remove_by_difficulty(difficulty_t difficulty);

static game_state_t game_state;
static game_screen_state_t current_screen_state = GAME_STATE_INTRO;
static difficulty_t selected_difficulty;

static bool show_help = false;

static unsigned intro_animation_time = 0;
static bool intro_animation_done = false;
static bool intro_text_shown = false;   // currently unused, kept for future use

static float cursor_x = 4.f;
static float cursor_y = 4.f;
static bool cursor_moving = false;
static const float lerp_speed = .2f;
static const float snap_threshold = .05f;
static unsigned blink_start_time = 0;

void game_init() {
    memset(&game_state, 0, sizeof(game_state_t));

    current_screen_state = GAME_STATE_INTRO;
    intro_animation_time = 0;
    intro_animation_done = false;
    intro_text_shown = false;
}

void game_new_puzzle(difficulty_t difficulty) {
    game_state.difficulty = difficulty;
    game_state.cursor_row = game_state.cursor_col = 4;
    cursor_x = cursor_y = 4.0f;
    game_state.selected_color = 0;
    game_state.solved = false;
    blink_start_time = time_us_32() / 1000000;

    const int cells_to_remove = cells_to_remove_by_difficulty(difficulty);

    clear(&game_state.puzzle);
    solve_puzzle(&game_state.puzzle);
    create_puzzle_from_solution(&game_state.puzzle, cells_to_remove);

    game_state.start_time = time_us_32() / 1000000;
    game_state.elapsed_time = 0;

    // Load best time from EEPROM index 0
    high_score_t hs;
    if (eeprom_read_high_score(0, &hs) &&
        hs.score != 0xFFFFFFFFu &&
        hs.score != 0u) {
        game_state.best_time = hs.score;
    } else {
        game_state.best_time = 0;  // no record yet
    }

    // LCD1: show difficulty briefly
    const char *diff_str = DIFFICULTY_NAMES[game_state.difficulty];
    display_show_difficulty(diff_str);

    // LCD2: show HUD (best time + help hint)
    display2_show_game_hud(game_state.best_time);
}

void game_update() {
    uint32_t current_time = time_us_32() / 1000000;

    // INTRO / MENU STATE
    if (current_screen_state == GAME_STATE_INTRO ||
        current_screen_state == GAME_STATE_MENU) {

        // Always drive the intro visuals
        draw_intro_screen();

        // Check for difficulty selection (1, 2, or 3)
        while (1) {
            uint16_t event = keypad_get_event();
            if (event == 0) break;

            if (keypad_is_pressed(event)) {
                char key = keypad_get_char(event);

                if (key == '1' && intro_animation_done) {
                    selected_difficulty = DIFFICULTY_EASY;
                    current_screen_state = GAME_STATE_PLAYING;
                    game_new_puzzle(selected_difficulty);
                    intro_animation_time = 0;
                    intro_animation_done = false;
                    intro_text_shown = false;
                    break;
                } else if (key == '2' && intro_animation_done) {
                    selected_difficulty = DIFFICULTY_MEDIUM;
                    current_screen_state = GAME_STATE_PLAYING;
                    game_new_puzzle(selected_difficulty);
                    intro_animation_time = 0;
                    intro_animation_done = false;
                    intro_text_shown = false;
                    break;
                } else if (key == '3' && intro_animation_done) {
                    selected_difficulty = DIFFICULTY_HARD;
                    current_screen_state = GAME_STATE_PLAYING;
                    game_new_puzzle(selected_difficulty);
                    intro_animation_time = 0;
                    intro_animation_done = false;
                    intro_text_shown = false;
                    break;
                }
            }
        }
        return;
    }

    // PLAYING STATE
    if (current_screen_state == GAME_STATE_PLAYING) {
        // Update time only while unsolved
        if (!game_state.solved) {
            game_state.elapsed_time = current_time - game_state.start_time;
        }

        // LCD1: timer + mode
        display_show_timer(game_state.elapsed_time);
        display_show_mode(DIFFICULTY_NAMES[game_state.difficulty]);

        // Handle keypad input for moves
        game_handle_keypad();
        game_handle_joystick();

        // Smooth cursor interpolation
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

        // Check solved
        if (!game_state.solved && game_check_solved()) {
            game_state.solved = true;
            audio_play_victory_tune();

            // Final time is the current elapsed_time (already frozen above)
            uint32_t final_time = game_state.elapsed_time;

            // Determine current best from EEPROM
            high_score_t hs;
            bool has_valid_best =
                eeprom_read_high_score(0, &hs) &&
                hs.score != 0xFFFFFFFFu &&
                hs.score != 0u;

            uint32_t old_best = has_valid_best ? hs.score : 0xFFFFFFFFu;
            bool new_record = !has_valid_best || (final_time < old_best);

            // If new record, write to EEPROM
            if (new_record) {
                high_score_t new_hs;
                new_hs.score = final_time;
                // optional name "CHROMA"
                for (int i = 0; i < 16; ++i) {
                    new_hs.name[i] = 0;
                }
                new_hs.name[0] = 'C';
                new_hs.name[1] = 'H';
                new_hs.name[2] = 'R';
                new_hs.name[3] = 'O';
                new_hs.name[4] = 'M';
                new_hs.name[5] = 'A';
                eeprom_write_high_score(0, &new_hs);

                game_state.best_time = final_time;
            }

            uint32_t best_to_show = new_record ? final_time : old_best;

            // LCD1: show final vs best
            display_show_final_and_best(final_time, best_to_show);

            // LCD2: solved message (with or without new high score)
            display2_show_solved(new_record);
        }

        // Draw board to panel
        game_draw_board();
    }
}

void game_handle_keypad() {
    show_help = keypad_is_key_held('*');

    while (1) {
        uint16_t keypad_event = keypad_get_event();
        uint32_t joystick_event = joystick_get_event();
        if (keypad_event == 0) {
            break;
        }

        if (keypad_is_pressed(keypad_event)) {
            char key = keypad_get_char(keypad_event);

            switch (key) {
            case '0':
                set(&game_state.puzzle,
                    game_state.cursor_row,
                    game_state.cursor_col,
                    0);
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
                set(&game_state.puzzle,
                    game_state.cursor_row,
                    game_state.cursor_col,
                    game_state.selected_color + 1);
                break;
            default:
                break;
            }
        }
    }
}

void game_handle_joystick() {
    const uint32_t event = joystick_get_event();
    const direction_t direction = joystick_is_pressed(event);

    if (direction & DIRECTION_N) {
        if (game_state.cursor_col > 0) {
            game_state.cursor_col--;
        }
    }

    if (direction & DIRECTION_E) {
        if (game_state.cursor_row < 8) {
            game_state.cursor_row++;
        }
    }

    if (direction & DIRECTION_W) {
        if (game_state.cursor_row > 0) {
            game_state.cursor_row--;
        }
    }

    if (direction & DIRECTION_S) {
        if (game_state.cursor_col < 8) {
            game_state.cursor_col++;
        }
    }
}

bool game_check_solved() {
    return is_valid(&game_state.puzzle);
}

void game_draw_board() {
    if (show_help) {
        draw_help_screen();
        hub75_refresh();
        return;
    }

    hub75_clear();
    draw_sudoku_puzzle(&game_state.puzzle);

    uint32_t current_time = time_us_32() / 1000000;
    uint32_t time_since_move = current_time - blink_start_time;
    bool show_cursor = cursor_moving || ((time_since_move % 2) == 0);

    if (show_cursor) {
        draw_cursor_ring(cursor_y, cursor_x);
    }

    hub75_refresh();
}

static void draw_sudoku_puzzle(sudoku_puzzle_t *puzzle) {
    for (uint8_t row = 0; row < 9; row++) {
        for (uint8_t col = 0; col < 9; col++) {
            uint8_t value = get(puzzle, row, col);
            color_t color = number_to_color(value);
            draw_sudoku_cell(row, col, color);
        }
    }
}

static void draw_sudoku_cell(uint8_t row, uint8_t col, color_t color) {
    uint8_t start_x, start_y;
    get_cell_position(row, col, &start_x, &start_y);

    for (uint8_t dy = 0; dy < 2; ++dy) {
        for (uint8_t dx = 0; dx < 2; ++dx) {
            hub75_set_pixel(start_x + dx,
                            start_y + dy,
                            color.r,
                            color.g,
                            color.b);
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
        if (x >= 0 && x < HUB75_PANEL_WIDTH &&
            start_y + 2 < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(x, start_y + 2, 255, 255, 255);
        }
    }
    for (int16_t y = start_y; y <= start_y + 1; y++) {
        if (start_x - 1 >= 0 && y >= 0 && y < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(start_x - 1, y, 255, 255, 255);
        }
    }
    for (int16_t y = start_y; y <= start_y + 1; y++) {
        if (start_x + 2 < HUB75_PANEL_WIDTH && y >= 0 &&
            y < HUB75_PANEL_HEIGHT) {
            hub75_set_pixel(start_x + 2, y, 255, 255, 255);
        }
    }
}

static void draw_intro_screen() {
    uint32_t current_time_ms = time_us_32() / 1000;

    // Keep both LCDs showing splash during intro/menu
    display_show_splash();
    display2_show_splash();

    if (intro_animation_time == 0) {
        intro_animation_time = current_time_ms;
    }

    uint32_t elapsed = current_time_ms - intro_animation_time;

    // Phase 1: Color rush animation
    if (elapsed < 2000) {
        draw_color_rush_animation(elapsed);
        hub75_refresh();
        return;
    }

    // Phase 2: Difficulty menu on panel
    hub75_clear();

    draw_char_5x7('1', 1, 3, COLOR_CYAN);
    draw_text("EASY", 8, 3, COLOR_WHITE);

    draw_char_5x7('2', 1, 13, COLOR_ORANGE);
    draw_text("MED", 8, 13, 255, 255, 255);

    draw_char_5x7('3', 1, 23, COLOR_RED);
    draw_text("HARD", 8, 23, 255, 255, 255);

    hub75_refresh();
    intro_animation_done = true;
}

static void draw_help_screen() {
    hub75_clear();

    for (uint8_t i = 0; i < 9; i++) {
        uint8_t row = i / 3;
        uint8_t col = i % 3;
        uint8_t start_x = 2 + col * 10;
        uint8_t start_y = 2 + row * 10;

        uint8_t digit = i + 1;
        draw_char_5x7(digit + '0',
                      start_x,
                      start_y,
                      color_map[digit - 1].r,
                      color_map[digit - 1].g,
                      color_map[digit - 1].b);
    }
}

static void draw_color_rush_animation(uint32_t time_ms) {
    hub75_clear();

    for (uint8_t y = 0; y < HUB75_PANEL_HEIGHT; y++) {
        for (uint8_t x = 0; x < HUB75_PANEL_WIDTH; x++) {
            float wave_pos = (float)(x + y + time_ms / 10) * 0.3f;
            float hue = fmodf(wave_pos, 6.0f);

            uint8_t r, g, b;

            if (hue < 1.0f) {
                r = 255;
                g = (uint8_t)(hue * 255);
                b = 0;
            } else if (hue < 2.0f) {
                r = (uint8_t)((2.0f - hue) * 255);
                g = 255;
                b = 0;
            } else if (hue < 3.0f) {
                r = 0;
                g = 255;
                b = (uint8_t)((hue - 2.0f) * 255);
            } else if (hue < 4.0f) {
                r = 0;
                g = (uint8_t)((4.0f - hue) * 255);
                b = 255;
            } else if (hue < 5.0f) {
                r = (uint8_t)((hue - 4.0f) * 255);
                g = 0;
                b = 255;
            } else {
                r = 255;
                g = 0;
                b = (uint8_t)((6.0f - hue) * 255);
            }

            float fade = 1.0f;
            if (time_ms < 500) {
                fade = (float)time_ms / 500.0f;
            } else if (time_ms > 1500) {
                fade = 1.0f - ((float)(time_ms - 1500) / 500.0f);
                if (fade < 0) fade = 0;
            }

            r = (uint8_t)(r * fade);
            g = (uint8_t)(g * fade);
            b = (uint8_t)(b * fade);

            hub75_set_pixel(x, y, r, g, b);
        }
    }
}

static void get_cell_position(uint8_t row, uint8_t col, uint8_t *x, uint8_t *y) {
    *x = 2 + (col / 3) + (col * 3);
    *y = 2 + (row / 3) + (row * 3);
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

static void create_puzzle_from_solution(sudoku_puzzle_t *puzzle,
                                        int cells_to_remove) {
    memcpy(puzzle->solution, puzzle->grid, 81);

    uint8_t positions[81];
    for (int i = 0; i < 81; i++) {
        positions[i] = i;
    }
    shuffle_array(positions, 81);

    hub75_clear();

    int removed = 0;
    for (int i = 0; i < 81 && removed < cells_to_remove; i++) {
        sleep_ms(10);
        int pos = positions[i];
        int row = pos / 9;
        int col = pos % 9;

        uint8_t backup = get(puzzle, row, col);
        set(puzzle, row, col, 0);

        if (has_unique_solution(puzzle)) {
            removed++;
        } else {
            set(puzzle, row, col, backup);
        }

        // Show progressive carving on panel
        draw_sudoku_puzzle(puzzle);
        hub75_refresh();
    }
}

static unsigned cells_to_remove_by_difficulty(difficulty_t difficulty) {
    switch (difficulty) {
    case DIFFICULTY_EASY:
        return 36;
    case DIFFICULTY_MEDIUM:
        return 42;
    case DIFFICULTY_HARD:
        return 50;
    default:
        return cells_to_remove_by_difficulty(DIFFICULTY_DEFAULT);
    }
}
