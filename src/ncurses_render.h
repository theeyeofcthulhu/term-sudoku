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

#include "util.h"
#include "sudoku.h"
#include "main.h"

extern bool render_small_mode;
extern char* statusbar;

struct cursor{
	int y;
	int x;
};

extern struct cursor cursor;

void init_ncurses();
void draw();
void read_sudoku(char* sudoku, int color_mode);
void move_cursor_to(int x, int y);
void move_cursor();
void draw_sudokus();
void read_notes();
