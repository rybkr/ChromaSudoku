#ifndef SUDOKU_H_416AAA1E2ECC5CA3
#define SUDOKU_H_416AAA1E2ECC5CA3

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    uint8_t grid[81];
    uint8_t solution[81];
} sudoku_puzzle_t;

void clear(sudoku_puzzle_t *puzzle);

uint8_t get(sudoku_puzzle_t *puzzle, int row, int col);
void set(sudoku_puzzle_t *puzzle, int row, int col, uint8_t value);

bool is_valid_placement(sudoku_puzzle_t *puzzle, int row, int col, uint8_t num);
bool is_valid(sudoku_puzzle_t *puzzle);

bool find_empty_cell(sudoku_puzzle_t *puzzle, int *row, int *col);

bool solve_puzzle(sudoku_puzzle_t *puzzle);
void fill_diagonal_boxes(sudoku_puzzle_t *puzzle);
bool has_unique_solution(sudoku_puzzle_t *puzzle);

void create_puzzle_from_solution(sudoku_puzzle_t *puzzle, int cells_to_remove);


#endif // SUDOKU_H_416AAA1E2ECC5CA3
