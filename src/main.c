#ifndef NOPICO
#include "pico/stdlib.h"
#endif
#include <stdio.h>
#include "sudoku.h"
#include "game.h"
#include "audio.h"
#include "display.h"
#include "eeprom.h"
#include "keypad.h"
#include "hub75.h"
#include "hardware/gpio.h"

void pretty_print(sudoku_puzzle_t *puzzle)
{
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

int main()
{
#ifndef NOPICO
    stdio_init_all();
#endif

    printf("\n\n========= RP2350 Chroma Sudoku =========\n");
#ifndef NOPICO
    printf("\nInitializing hardware...\n");
    
    // Initialize all subsystems
    audio_init();
    display_init();
    display_show_splash(); //possible screen before game screen
    eeprom_init();
    keypad_init();
    hub75_init();
    
    printf("Hardware initialized\n");
    printf("Starting game...\n");
    
    // Initialize game
    game_init();
    game_new_puzzle(DIFFICULTY_HARD);
    
    // Main game loop
    for (;;) {
        game_update();
    }
#else
    // Test mode without hardware
    clock_t start = clock(), diff;

    sudoku_puzzle_t puzzle;
    clear(&puzzle);
    pretty_print(&puzzle);

    fill_diagonal_boxes(&puzzle);
    pretty_print(&puzzle);

    solve_puzzle(&puzzle);
    pretty_print(&puzzle);

    create_puzzle_from_solution(&puzzle, 51);
    pretty_print(&puzzle);

    diff = clock() - start;
    printf("Puzzle generated in %lu milliseconds\n", diff * 1000 / CLOCKS_PER_SEC);
#endif

    return 0;
}
