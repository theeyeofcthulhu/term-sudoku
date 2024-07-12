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

#include "util.h"

#include "main.h"
#include "ncurses_render.h"
#include "sudoku.h"

#include <curses.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void fprintf_char_arr(const char *arr, size_t sz, FILE *out);
void fprintf_int_arr(const int *arr, size_t sz, FILE *out);

// Exit ncurses cleanly

void finish(int sig)
{
    endwin();
    if (sig == SIGSEGV) {
        printf("Segfault\n");
        exit(1);
    }
    exit(0);
}

// Takes the same parameters as printf and exits ncurses as well as the
// application
void finish_with_err_msg(const char *msg, ...)
{
    endwin();

    // Send '...'-args to vprintf
    va_list format;
    va_start(format, msg);
    vfprintf(stderr, msg, format);
    va_end(format);

    exit(1);
}

void finish_with_errno(const char *msg, ...) {
    endwin();

    va_list format;
    va_start(format, msg);
    vfprintf(stderr, msg, format);
    va_end(format);

    fprintf(stderr, ": %s\n", strerror(errno));

    exit(1);
}

// List files in a directory into malloc'd array of malloc'd strings (iterator will be returned as the size of items)
char **listfiles(const char *dir_name, int *iterator)
{
    // Set iterator
    *iterator = 0;

    DIR *dirp = opendir(dir_name);

    struct dirent *entry;
    int files = 0;

    char **ret;

    // Load contents of directory into items
    if (!dirp) {
        finish_with_errno(dir_name);
    }

    // get size
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            files++;
        }
    }

    ret = malloc(files * sizeof(char *));

    // get contents
    rewinddir(dirp);
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            ret[(*iterator)++] = strdup(entry->d_name);
        }
    }

    closedir(dirp);

    return ret;
}

void freefiles(char **files, int sz)
{
    for (int i = 0; i < sz; i++)
        free(files[i]);
    free(files);
}

void fprintf_char_arr(const char *arr, size_t sz, FILE *out)
{
    for (size_t i = 0; i < sz; i++) {
        fprintf(out, "%c", arr[i]);
    }
    fprintf(out, "\n");
}

void fprintf_int_arr(const int *arr, size_t sz, FILE *out)
{
    for (size_t i = 0; i < sz; i++) {
        fprintf(out, "%d", arr[i]);
    }
    fprintf(out, "\n");
}

// Write sudoku_str, user_nums and notes to file
bool savestate(const char *filename, const struct SudokuSpec *spec)
{
    FILE *savestate = fopen(filename, "w");

    if (savestate == NULL)
        return false;

    fprintf_char_arr(spec->sudoku, SUDOKU_LEN, savestate);
    fprintf_char_arr(spec->user, SUDOKU_LEN, savestate);
    fprintf_int_arr(spec->notes, SUDOKU_LEN * LINE_LEN, savestate);

    fclose(savestate);

    return true;
}

void gen_file_name(char *filename, size_t sz, char *dir)
{
    time_t t = time(NULL);
    // localtime struct for file name
    struct tm tm = *localtime(&t);

    // Generate file name with the current time
    char fn[STR_LEN];
    strftime(fn, sizeof(fn), "%Y-%m-%d-%H-%M-%S.sudoku", &tm);

    snprintf(filename, sz, "%s/%s", dir, fn);
}

// Ask for confirmation by displaying
bool status_bar_confirmation(struct TSStruct *spec)
{
    // Return if the '-c' flag is set (the user does not want to be asked)
    if (!spec->opts->ask_confirmation)
        return true;

    char statusbar_backup[STR_LEN];
    strcpy(statusbar_backup, spec->statusbar);

    sprintf(spec->statusbar, "%s", "Sure? y/n");
    draw(spec);

    char confirm = getch();

    sprintf(spec->statusbar, "%s", statusbar_backup);
    draw(spec);

    return confirm == 'y';
}
