#include "rng.h"
#include "string.h"
#include "sudoku.h"

void create_puzzle_from_solution(sudoku_puzzle_t *puzzle, int cells_to_remove) {
    memcpy(puzzle->solution, puzzle->grid, 81);

    uint8_t positions[81];
    for (int i = 0; i < 81; i++) {
        positions[i] = i;
    }
    shuffle_array(positions, 81);

    int removed = 0;
    for (int i = 0; i < 81 && removed < cells_to_remove; i++) {
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
    }
}
