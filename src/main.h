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

#include <stdbool.h>

#define LINE_LEN 9
#define SUDOKU_LEN 81
#define SOLUTION_SUM ((((LINE_LEN*LINE_LEN)+LINE_LEN)/2)*LINE_LEN) // sum of all numbers in a correct solution
#define ATTEMPTS_DEFAULT 5
#define STR_LEN 80
#define PUZZLE_OFFSET 1

struct TSOpts{
    bool gen_visual;
    bool own_sudoku;
    int attempts;
    char *dir;
    bool from_file;
    bool ask_confirmation;
    bool small_mode;
    char filename[STR_LEN];
};

struct TSStruct {
    const char *controls;
    char statusbar[STR_LEN];
    int highlight;
    bool editing_notes;
    struct SudokuSpec *sudoku;
    struct TSOpts *opts;
    struct Cursor *cursor;
};

