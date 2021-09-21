/*
term-sudoku: play sudoku in the terminal
Copyright (C) 2021 eyeofcthulhu

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

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>

#define LINE_LEN 9
#define SUDOKU_LEN 81
#define ATTEMPTS_DEFAULT 5
#define VISUAL_SLEEP 10000
#define STR_LEN 80

extern bool sudoku_gen_visual;
extern int sudoku_attempts;

struct sudoku_cell_props;

struct sudoku_cell_props get_cell_props(int cell, char* sudoku_str);
void free_cell_props(struct sudoku_cell_props cell);
void generate_sudoku(char* gen_sudoku);
int fill_remaining(int start);
void remove_nums(char* gen_sudoku);
bool solve(char* sudoku_str);
void solve_count(char* sudoku_to_solve, int* count);
bool solve_user_nums(char* sudoku_str, char* user_nums);
void generate_visually(char* sudoku_to_display);
bool check_validity(char* combined_solution);
