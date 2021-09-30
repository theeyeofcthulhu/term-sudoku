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

#include <curses.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "ncurses_render.h"
#include "main.h"

extern bool util_ask_confirmation;

void finish(int sig);
void finish_with_err_msg(char* msg);
void listfiles(char* target_dir, char* items[], int* iterator);
bool savestate(char* filename, char* sudoku_str, char* user_nums, int* notes);
bool status_bar_confirmation();
