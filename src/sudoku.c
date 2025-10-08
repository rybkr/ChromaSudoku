#include "sudoku.h"

static inline int get_index(int row, int col)
{
    return row * 9 + col;
}

uint8_t get(sudoku_puzzle_t *puzzle, int row, int col)
{
    return puzzle->grid[get_index(row, col)];
}

void set(sudoku_puzzle_t *puzzle, int row, int col, uint8_t value)
{
    puzzle->grid[get_index(row, col)] = value;
}

void clear(sudoku_puzzle_t *puzzle)
{
    for (int r = 0; r < 9; ++r) for (int c = 0; c < 9; ++c) {
        set(puzzle, r, c, 0);
    }
}

bool is_valid_placement(sudoku_puzzle_t *puzzle, int row, int col, uint8_t num)
{
    for (int c = 0; c < 9; c++) if (c != col && get(puzzle, row, c) == num) {
        return false;
    }
    
    for (int r = 0; r < 9; r++) if (r != row && get(puzzle, r, col) == num) {
        return false;
    }
    
    int box_row = (int)(row / 3) * 3;
    int box_col = (int)(col / 3) * 3;
    for (int r = box_row; r < box_row + 3; r++) for (int c = box_col; c < box_col + 3; c++) if ((c != col || r != row) && get(puzzle, r, c) == num) {
        return false;
    }
    
    return true;
}

bool is_valid(sudoku_puzzle_t *puzzle)
{
    for (int r = 0; r < 9; ++r) for (int c = 0; c < 9; ++c) if (!is_valid_placement(puzzle, r, c, get(puzzle, r, c))) {
        return false;
    }
    return true;
}

bool find_empty_cell(sudoku_puzzle_t *puzzle, int *row, int *col)
{
    for (int r = 0; r < 9; r++) for (int c = 0; c < 9; c++) if (get(puzzle, r, c) == 0) {
        *row = r;
        *col = c;
        return true;
    }
    return false;
}
