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

struct Cursor {
    int y;
    int x;
};

void init_ncurses(void);
void draw(const struct TSStruct *spec);
void move_cursor_to(struct Cursor *curs, bool small_mode, int x, int y);
void move_cursor(struct Cursor *curs, bool small_mode);
void init_visual_generator(struct TSStruct *spec);
void generate_visually(const char *sudoku_to_display);
