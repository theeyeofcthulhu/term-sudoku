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

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdbool.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>

#include "util.h"
#include "sudoku.h"
#include "ncurses_render.h"

extern char* controls;

#define LINE_LEN 9
#define SUDOKU_LEN 81
#define ATTEMPTS_DEFAULT 5
#define VISUAL_SLEEP 10000
#define STR_LEN 80
#define PUZZLE_OFFSET 1

void input_go_to();
