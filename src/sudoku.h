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

#include <stdbool.h>

extern char user_nums[SUDOKU_LEN];
extern char sudoku_str[SUDOKU_LEN];
extern int notes[SUDOKU_LEN * LINE_LEN];

void generate_sudoku(char *gen_sudoku);
bool solve_user_nums();
bool check_validity(const char *sudoku_to_check);
