/*
term-sudoku: play sudoku in the terminal
Copyright (C) 2024 eyeofcthulhu

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "sudoku.h"

#include "main.h"
#include "ncurses_render.h"
#include "util.h"

#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void solve_count(char *sudoku_to_solve, int *count);
void remove_nums(char *gen_sudoku, const struct TSOpts *opts);

#define COLUMN(sudoku, cell, i) ((sudoku)[(i) * LINE_LEN + ((cell) % LINE_LEN)])
#define ROW(sudoku, cell, i) ((sudoku)[((cell) / LINE_LEN) * LINE_LEN + (i)])
#define BLOCK(sudoku, cell, i) ((sudoku)[((((cell) / LINE_LEN) / 3) * 3) * LINE_LEN + \
                                         ((((cell) % LINE_LEN) / 3) * 3) + \
                                         (LINE_LEN * ((i) / 3)) + ((i) % 3)])

// Generate a random sudoku
// This function generates the diagonal blocks from left to right and then calls
// solve() and remove_nums() to first fill out and then remove some numbers to
// create a complete puzzle
void generate_sudoku(char *gen_sudoku, const struct TSOpts *opts)
{
    if (opts->gen_visual)
        curs_set(0);
    // Fill each diagonal block with the values 1-9
    /* [x][ ][ ]
     * [ ][x][ ]
     * [ ][ ][x] */
    for (int i = 0; i < 3; i++) {
        bool used[LINE_LEN] = { false };

        for (int j = 0; j < LINE_LEN; j++) {
            int num;
            do {
                num = rand() % 9;
            } while (used[num]);

            used[num] = true;

            gen_sudoku[(i * 3) * LINE_LEN + (i * 3) +
                       (LINE_LEN * (j / 3)) + (j % 3)] = '1' + num;

            // Draw the process of filling out the sudoku visually on the screen
            // if that option is set via '-v'
            if (opts->gen_visual) {
                generate_visually(gen_sudoku);
            }
        }
    }

    // Solve the remaining blocks
    solve(gen_sudoku, opts->gen_visual);
    // Remove numbers but maintain unique solution
    remove_nums(gen_sudoku, opts);

    if (opts->gen_visual)
        curs_set(1);
}

// Try and remove numbers until the solution is not unique
void remove_nums(char *gen_sudoku, const struct TSOpts *opts)
{
    int local_attempts = opts->attempts;
    // Run down the attempts defined with '-n'
    while (local_attempts > 0) {
        // Get non-empty cell
        int cell = -1;
        while (cell < 0) {
            cell = rand() % SUDOKU_LEN;
            if (gen_sudoku[cell] == '0')
                cell = -1;
        }

        // Generate a copy of the sudoku and count how many solutions it has
        char sudoku_cpy[SUDOKU_LEN];
        memcpy(sudoku_cpy, gen_sudoku, SUDOKU_LEN);

        sudoku_cpy[cell] = '0';
        int count = 0;
        solve_count(sudoku_cpy, &count);

        // If unique, apply to real sudoku
        if (count == 1) {
            gen_sudoku[cell] = '0';
            if (opts->gen_visual)
                generate_visually(gen_sudoku);
        }
        // Else, burn an attempt
        else {
            local_attempts--;
        }
    }
}

// Solve a sudoku (used in generating)
bool solve(char *sudoku_to_solve, bool visual)
{
    // Draw the process of filling out the sudoku visually on the screen if that
    // option is set via '-v'
    if (visual)
        generate_visually(sudoku_to_solve);

    // If sudoku is valid, return
    if (check_validity(sudoku_to_solve))
        return true;

    for (int i = 0; i < SUDOKU_LEN; i++) {
        if (sudoku_to_solve[i] == '0') {
            // Try to assign a value to the cell at i
            for (int j = '1'; j <= '9'; j++) {
                bool used = false;
                for (int k = 0; k < LINE_LEN && !used; k++) {
                    // Check if the value is valid
                    if (COLUMN(sudoku_to_solve, i, k) == j ||
                        ROW(sudoku_to_solve, i, k) == j ||
                        BLOCK(sudoku_to_solve, i, k) == j)
                    {
                        used = true;
                    }
                }
                if (!used) {
                    sudoku_to_solve[i] = j;
                    // Check the whole path
                    if (solve(sudoku_to_solve, visual)) {
                        return true;
                    }

                    // Otherwise, go back to 0
                    sudoku_to_solve[i] = '0';
                }
            }

            break;
        }
    }
    return false;
}

// Count the solutions to a puzzle
// Go through the puzzle recursively and increase count everytime you find a
// solution
void solve_count(char *sudoku_to_solve, int *count)
{
    // Function only needs to check if there is more than one unique
    // solution, so return if there is
    if (*count > 1)
        return;

    // find empty cell
    for (int i = 0; i < SUDOKU_LEN; i++) {
        if (sudoku_to_solve[i] == '0') {
            // Try to assign a value to the cell at i
            for (int j = '1'; j <= '9'; j++) {
                bool used = false;
                for (int k = 0; k < LINE_LEN && !used; k++) {
                    // Check if the value is valid
                    if (COLUMN(sudoku_to_solve, i, k) == j ||
                        ROW(sudoku_to_solve, i, k) == j ||
                        BLOCK(sudoku_to_solve, i, k) == j)
                    {
                        used = true;
                    }
                }
                if (!used) {
                    sudoku_to_solve[i] = j;
                    // If assigning this value solved the grid, increase the
                    // count
                    if (check_validity(sudoku_to_solve)) {
                        *count += 1;
                        sudoku_to_solve[i] = '0';
                        break;
                    }
                    // If not solved recursively call
                    else {
                        // Check the whole path
                        solve_count(sudoku_to_solve, count);
                    }

                    // Otherwise, go back to 0
                    sudoku_to_solve[i] = '0';
                }
            }
            break;
        }
    }
}

// Check for errors in the solved sudoku
bool check_validity(const char *combined_solution)
{
    /* Check first if it's possible that the solution is correct
     * by checking if the values in it add up to nine times the sum
     * of the nine digits (405) */
    int sum = 0;
    for (int i = 0; i < SUDOKU_LEN; i++) {
        sum += CHNUM(combined_solution[i]);
    }
    if (sum != SOLUTION_SUM) {
        return false;
    }


    // Go through all nine columns, rows and blocks and check for
    // duplicates
    for (int i = 0; i < LINE_LEN; i++) {
        bool appeared[3][LINE_LEN + 1] = {false};

        for (int j = 0; j < LINE_LEN; j++) {
            int cur_comp[3];

            cur_comp[0] = CHNUM(COLUMN(combined_solution, i, j));
            cur_comp[1] = CHNUM(ROW(combined_solution, i * LINE_LEN, j));
            // evaluates to top-left cell of i'th block
            cur_comp[2] = CHNUM(BLOCK(combined_solution, ((i / 3) * 3) + (((i % 3) * 3) * LINE_LEN), j));

            // Check if j'th number in column, row or block already appeared
            for (int k = 0; k < 3; k++) {
                if (appeared[k][cur_comp[k]])
                    return false;

                appeared[k][cur_comp[k]] = true;
            }
        }
    }

    return true;
}
