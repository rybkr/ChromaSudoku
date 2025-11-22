#include "audio.h"
#include "display.h"
#include "game.h"
#include "sudoku.h"
#include "eeprom.h"
#include "hub75.h"
#include "keypad.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdbool.h>

void pretty_print(sudoku_puzzle_t *puzzle) {
    for (int r = 0; r < 9; ++r) {
        if (r % 3 == 0) {
            printf("+-------+-------+-------+\n");
        }
        printf("| ");
        for (int c = 0; c < 9; c++) {
            if (get(puzzle, r, c) == 0) {
                printf(". ");
            } else {
                printf("%d ", (int)get(puzzle, r, c));
            }
            if ((c + 1) % 3 == 0) {
                printf("| ");
            }
        }
        printf("\n");
    }
    printf("+-------+-------+-------+\n\n");
}

int main() {
    stdio_init_all();

    printf("\n\n========= RP2350 Chroma Sudoku =========\n");
    printf("\nInitializing hardware...\n");

    audio_init();
    display_init();
    display_show_splash(); // possible screen before game screen
    eeprom_init();
    keypad_init();
    hub75_init();

    game_init();
    game_new_puzzle(DIFFICULTY_EASY);

    while (true) {
        game_update();
    }

    return 0;
}
