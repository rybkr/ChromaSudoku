#include "rng.h"
#include "sudoku.h"
#include <string.h>

bool solve_puzzle(sudoku_puzzle_t *puzzle) {
    int row, col;

    if (!find_empty_cell(puzzle, &row, &col)) {
        return true;
    }

    uint8_t numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffle_array(numbers, 9);

    for (int i = 0; i < 9; ++i) {
        uint8_t num = numbers[i];

        if (is_valid_placement(puzzle, row, col, num)) {
            set(puzzle, row, col, num);

            if (solve_puzzle(puzzle)) {
                return true;
            }

            set(puzzle, row, col, 0);
        }
    }

    return false;
}

void fill_diagonal_boxes(sudoku_puzzle_t *puzzle) {
    for (int box = 0; box < 9; box += 3) {
        uint8_t numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        shuffle_array(numbers, 9);

        int idx = 0;
        for (int r = box; r < box + 3; r++) {
            for (int c = box; c < box + 3; c++) {
                set(puzzle, r, c, numbers[idx++]);
            }
        }
    }
}

static int solution_count;
static int max_solutions;

static bool count_solutions_helper(sudoku_puzzle_t *puzzle) {
    if (solution_count >= max_solutions) {
        return false;
    }

    int row, col;
    if (!find_empty_cell(puzzle, &row, &col)) {
        solution_count++; // Found a complete solution
        return (solution_count < max_solutions);
    }

    for (uint8_t num = 1; num <= 9; num++) {
        if (is_valid_placement(puzzle, row, col, num)) {
            set(puzzle, row, col, num);

            if (!count_solutions_helper(puzzle)) {
                set(puzzle, row, col, 0);
                return false;
            }

            set(puzzle, row, col, 0);
        }
    }

    return true;
}

static int count_solutions(sudoku_puzzle_t *puzzle, int max_to_find) {
    sudoku_puzzle_t temp;
    memcpy(temp.solution, puzzle->solution, 81);
    memcpy(temp.grid, puzzle->grid, 81);

    solution_count = 0;
    max_solutions = max_to_find;

    count_solutions_helper(&temp);

    return solution_count;
}

bool has_unique_solution(sudoku_puzzle_t *puzzle) {
    int n_solutions = count_solutions(puzzle, 2);
    return n_solutions == 1;
}
