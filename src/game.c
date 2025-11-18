#include "pico/stdlib.h"
#include "game.h"
#include "hub75.h"
#include "keypad.h"
#include "display.h"
#include "audio.h"
#include "eeprom.h"
#include <stdio.h>
#include <string.h>

static game_state_t game_state;

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

void game_init(void) {
    memset(&game_state, 0, sizeof(game_state_t));
    game_state.cursor_row = 0;
    game_state.cursor_col = 0;
    game_state.selected_color = 0;
    game_state.difficulty = DIFFICULTY_EASY;
    game_state.solved = false;
}

void game_new_puzzle(difficulty_t difficulty) {
    game_state.difficulty = difficulty;
    game_state.cursor_row = 0;
    game_state.cursor_col = 0;
    game_state.selected_color = 0;
    game_state.solved = false;
    
    int cells_to_remove;
    switch (difficulty) {
        case DIFFICULTY_EASY:
            cells_to_remove = 30;
            break;
        case DIFFICULTY_MEDIUM:
            cells_to_remove = 40;
            break;
        case DIFFICULTY_HARD:
            cells_to_remove = 51;
            break;
        default:
            cells_to_remove = 30;
    }
    
    clear(&game_state.puzzle);
    fill_diagonal_boxes(&game_state.puzzle);
    solve_puzzle(&game_state.puzzle);
    create_puzzle_from_solution(&game_state.puzzle, cells_to_remove);
    
    game_state.start_time = time_us_32() / 1000000;
    game_state.elapsed_time = 0;
    
    const char *diff_str;
    switch (difficulty) {
        case DIFFICULTY_EASY: diff_str = "Easy"; break;
        case DIFFICULTY_MEDIUM: diff_str = "Med"; break;
        case DIFFICULTY_HARD: diff_str = "Hard"; break;
        default: diff_str = "Easy";
    }
    display_show_difficulty(diff_str);

    high_score_t hs;
    if (eeprom_read_high_score(0, &hs) && hs.score != 0xFFFFFFFF && hs.score != 0) {
        display_high_score(hs.score);   // <-- your LCD function
    } else {
        display_high_score(0);          // Shows "Best: 00:00"
    }
}

void game_update(void) {
    uint32_t current_time = time_us_32() / 1000000;
    game_state.elapsed_time = current_time - game_state.start_time;
    
    display_show_timer(game_state.elapsed_time);

    if (!game_state.solved) {
        display2_show_instructions();
    }

    high_score_t hs;
    if (eeprom_read_high_score(0, &hs)) {
        display_high_score(hs.score);
    }
    
    game_handle_keypad();
    
    if (!game_state.solved && game_check_solved()) {
        game_state.solved = true;
        audio_play_victory_tune();

        //solved display and play again
        display_show_status("Solved!!");
        display2_clear(); //replace with play again later?

        // TODO(rybkr): Save high score if applicable
        uint32_t final_time = game_state.elapsed_time;

        high_score_t score = {
            .score = final_time,
        };

        // Check if this is a high score
        if (eeprom_is_high_score(final_time)) {
            eeprom_insert_high_score(&score);
            display_show_status("New Best!");
        } else {
            display_show_status("Solved!");
        }
    }
    
    game_draw_board();
}

void game_handle_keypad(void) {
    uint16_t event = keypad_get_event();
    if (event == 0) {
        return;
    }
    
    if (keypad_is_pressed(event)) {
        char key = keypad_get_char(event);
        
        if (key >= '1' && key <= '9') {
            game_state.selected_color = key - '1';
            
            uint8_t value = get(&game_state.puzzle, game_state.cursor_row, game_state.cursor_col);
            // TODO(rybkr): Check if this is a given cell (compare with solution)
            if (value == 0) {
                set(&game_state.puzzle, game_state.cursor_row, game_state.cursor_col, 
                    game_state.selected_color + 1);
            }
        }
        // TODO(rybkr): Handle other keys (e.g., clear, new game, etc.)
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
            bool selected = (row == game_state.cursor_row && col == game_state.cursor_col);
            hub75_draw_sudoku_cell(row, col, color, selected);
        }
    }
    
    hub75_update();
}
