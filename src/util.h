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
#include <stddef.h>

#define CHNUM(x) ((x) - 0x30)

struct SudokuSpec;
struct TSStruct;

void finish(int sig);
void finish_with_err_msg(const char *msg, ...);
void finish_with_errno(const char *msg, ...);
void gen_file_name(char *filename, size_t sz, char *dir);
char **listfiles(const char *dir_name, int *iterator);
void freefiles(char **files, int sz);
bool savestate(const char *filename, const struct SudokuSpec *spec);
bool status_bar_confirmation(struct TSStruct *spec);
