#ifndef NOPICO
#include "pico/stdlib.h"
#endif
#include <stdio.h>
#include <time.h>
#include "sudoku.h"

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
    printf("\nRunning on RP2350...\n");
#endif

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

    return 0;
}
