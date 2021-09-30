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

#pragma once

#include "main.h"

extern bool sudoku_gen_visual;
extern int sudoku_attempts;
extern char* user_nums;
extern char* sudoku_str;
extern int* notes;

struct sudoku_cell_props;

struct sudoku_cell_props get_cell_props(int cell, char* sudoku_str);
void init_sudoku_strings();
void free_cell_props(struct sudoku_cell_props cell);
void generate_sudoku(char* gen_sudoku);
void remove_nums(char* gen_sudoku);
bool solve(char* sudoku_str);
void solve_count(char* sudoku_to_solve, int* count);
bool solve_user_nums();
void generate_visually(char* sudoku_to_display);
bool check_validity(char* sudoku_to_check);
void new_sudoku(char* statusbar, char* filename, char* target_dir, time_t t);
