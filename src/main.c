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

#include "main.h"

#include "ncurses_render.h"
#include "sudoku.h"
#include "util.h"

#include <curses.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void new_sudoku(struct TSStruct *spec);
void input_go_to(struct TSStruct *spec);
bool own_sudoku_view(struct TSStruct *spec);
bool fileview(struct TSStruct *spec);
void mainloop(struct TSStruct *spec);

const char *controls_default = "move - h, j, k and l or arrow keys\n"
                               "1-9 - insert numbers\n"
                               "x or 0 - delete numbers\n"
                               "save - s\n"
                               "check for errors - c\n"
                               "solve sudoku - d\n"
                               "notetaking mode - e\n"
                               "go to position - g\n"
                               "highlight number - v\n"
                               "quit - q\n";
bool editing_notes = false;

const char *sharepath = ".local/share";
const char *appsharepath = ".local/share/term-sudoku";

// Generate a new sudoku for the user to solve
void new_sudoku(struct TSStruct *spec)
{
    struct TSOpts *opts = spec->opts;
    struct SudokuSpec *sudoku = spec->sudoku;

    gen_file_name(opts->filename, sizeof(opts->filename), opts->dir);

    // Clear sudoku arrays
    memset(sudoku->sudoku,  '0',    sizeof(sudoku->sudoku));
    memset(sudoku->user,    '0',    sizeof(sudoku->user));
    memset(sudoku->notes,    0,     sizeof(sudoku->notes));

    // Generate the sudoku
    generate_sudoku(sudoku->sudoku, opts);

    sprintf(spec->statusbar, "%s", "Sudoku generated");
}

// Ask for position (getch()) and go there
void input_go_to(struct TSStruct *spec)
{
    int move_to[2] = {0, 0};

    sprintf(spec->statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
    draw(spec);

    for (int i = 0; i < 2; i++) {
        char c_pos = getch();
        // TODO: CHAR_TO_NUM macro or something similar
        move_to[i] = c_pos - 0x30;
        if (move_to[i] <= 0 || move_to[i] > 9) {
            sprintf(spec->statusbar, "%s", "Cancelled");
            draw(spec);
            return;
        }

        sprintf(spec->statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
        draw(spec);
    }
    move_cursor_to(spec->cursor, spec->opts->small_mode, move_to[0] - 1, move_to[1] - 1);
}

bool own_sudoku_view(struct TSStruct *spec)
{
    spec->cursor->x = spec->cursor->y = 0;

    struct TSOpts *opts = spec->opts;
    struct SudokuSpec *sudoku = spec->sudoku;
    struct Cursor *curs = spec->cursor;

    gen_file_name(opts->filename, sizeof(opts->filename), opts->dir);

    // Clear sudoku arrays
    memset(sudoku->sudoku, '0', SUDOKU_LEN);
    memset(sudoku->user, '0', SUDOKU_LEN);
    memset(sudoku->notes, 0, sizeof(sudoku->notes));

    sprintf(spec->statusbar, "%s", "Enter your sudoku");
    // Controls displayed only in this view
    const char *custom_sudoku_controls = "move - h, j, k and l or arrow keys\n"
                                         "1-9 - insert numbers\n"
                                         "x or 0 - delete numbers\n"
                                         "done - d\n"
                                         "go to position - g\n"
                                         "quit - q\n";
    spec->controls = (char *)custom_sudoku_controls;
    // Draw with new controls
    draw(spec);
    bool done = false;
    bool quit = false;

    // Loop for entering own sudoku
    while (!done && !quit) {
        int key_press = getch();
        // Move on vim keys and bind to field size
        switch (key_press) {
        case KEY_LEFT:
        case 'h':
            curs->x = curs->x - 1 < 0 ? curs->x : curs->x - 1;
            goto move;
        case KEY_DOWN:
        case 'j':
            curs->y = curs->y + 1 >= LINE_LEN ? curs->y : curs->y + 1;
            goto move;
        case KEY_UP:
        case 'k':
            curs->y = curs->y - 1 < 0 ? curs->y : curs->y - 1;
            goto move;
        case KEY_RIGHT:
        case 'l':
            curs->x = curs->x + 1 >= LINE_LEN ? curs->x : curs->x + 1;
            goto move;
        move:
            move_cursor(curs, opts->small_mode);
            break;
        case 'd':
            if (!status_bar_confirmation(spec)) break;

            done = true;
            break;
        case 'g':
            input_go_to(spec);
            break;
        case 'q':
            if (!status_bar_confirmation(spec))
                break;

            quit = true;
            break;
        // Input numbers into the user sudoku field
        default:
            // Check if the key is a number (not zero) in aasci chars or 'x' and
            // if the cursor is not an a field filled by the puzzle

            // check for numbers
            if (key_press >= '1' && key_press <= '9' &&
                sudoku->sudoku[curs->y * LINE_LEN + curs->x] != key_press) {
                sudoku->sudoku[curs->y * LINE_LEN + curs->x] = key_press;
                draw(spec);
            }
            // check for x
            else if ((key_press == 'x' || key_press == '0') &&
                     sudoku->sudoku[curs->y * LINE_LEN + curs->x] != '0') {
                sudoku->sudoku[curs->y * LINE_LEN + curs->x] = '0';
                draw(spec);
            }
            break;
        }
    }
    // Reset controls
    spec->controls = (char *)controls_default;
    sprintf(spec->statusbar, "Sudoku entered");

    return !quit;
}

bool fileview(struct TSStruct *spec)
{
    // Array of strings (in this case: directories)
    char *items[STR_LEN];
    // Holds the size of 'items'
    int iterator;

    // Load files in directory into items
    listfiles(spec->opts->dir, items, &iterator);

    curs_set(0);

    bool chosen = false;
    bool new_file = false;
    bool own = false;

    static int position = 0;
    if (position > iterator)
        position = iterator - 1 >= 0 ? iterator - 1 : 0;

    const char *file_view_controls =
        "Choose a savegame - move - j and k, d - delete, confirm - y, new file "
        "- n, own sudoku - o, quit - q";

    // Choose file by moving cursor
    while (!chosen && !new_file && !own) {
        erase();

        mvprintw(0, 0, "%s", file_view_controls);

        for (int j = 0; j < iterator; j++)
            mvprintw(j + 2, 0, "  %s\n", items[j]);

        // Asteriks as cursor for file selection
        mvaddch(position + 2, 0, '*');

        int key_press = getch();

        // Move on vim keys and bind to item size later
        switch (key_press) {
        case KEY_DOWN:
        case 'j':
            position += 1;
            break;
        case KEY_UP:
        case 'k':
            position -= 1;
            break;
        case 'y':
            if (iterator > 0)
                chosen = true;
            break;
        case 'o':
            own = true;
            break;
        // Delete selected file and re-read files
        case 'd':
        {
            if(iterator <= 0)
                break;

            char temp_file[STR_LEN];
            snprintf(temp_file, STR_LEN, "%s/%s", spec->opts->dir, items[position]);
            if (remove(temp_file) == -1) {
                finish_with_errno("Removing %s", temp_file);
            }
            position = position - 1 >= 0 ? position - 1 : 0;

            for (int j = 0; j < iterator; j++)
                free(items[j]);
            listfiles(spec->opts->dir, items, &iterator);
            break;
        }
        case 'q':
            return false;
        // Create a new file instead of reading one
        case 'n':
            new_file = true;
            break;
        default:
            break;
        }

        if (iterator != 0) {
            // Wrap position according to list size
            if (position >= iterator)
                position = 0;
            else if (position < 0)
                position = iterator - 1;
            // Keep position 0 if no files are available
        } else {
            position = 0;
        }
    }

    curs_set(1);

    // Reading the file
    if (chosen) {
        snprintf(spec->opts->filename, STR_LEN, "%s/%s", spec->opts->dir, items[position]);

        for (int j = 0; j < iterator; j++)
            free(items[j]);

        // Read Sudoku from given file
        FILE *input_file = fopen(spec->opts->filename, "r");
        if (input_file == NULL) {
            finish_with_errno(spec->opts->filename);
        }

        // Scan in the first two lines which are the puzzle and the entered
        // user_nums
        fscanf(input_file, "%s\n%s\n", spec->sudoku->sudoku, spec->sudoku->user);
        // Read in notes into the array
        for (int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
            fscanf(input_file, "%1d", &spec->sudoku->notes[i]);
        fclose(input_file);

        sprintf(spec->statusbar, "%s", "File opened");

        mainloop(spec);
    } else if (own) {
        if (own_sudoku_view(spec))
            mainloop(spec);
    } else if (new_file) {
        new_sudoku(spec);
        mainloop(spec);
    }

    return true;
}

/*
** Draws the sudoku and processes input relating to modifying the sudoku,
*changing something about the rendering or moving the cursor
*/
void mainloop(struct TSStruct *spec)
{
    struct TSOpts *opts = spec->opts;
    struct SudokuSpec *sudoku = spec->sudoku;
    struct Cursor *curs = spec->cursor;

    curs->x = curs->y = 0;

    draw(spec);

    bool quit = false;
    // Main loop: wait for keypress, then process it
    while (!quit) {
        int key_press = getch();
        switch (key_press) {
        // Move on vim keys and bind to field size
        case KEY_LEFT:
        case 'h':
            curs->x = curs->x - 1 < 0 ? curs->x : curs->x - 1;
            goto move;
        case KEY_DOWN:
        case 'j':
            curs->y = curs->y + 1 >= LINE_LEN ? curs->y : curs->y + 1;
            goto move;
        case KEY_UP:
        case 'k':
            curs->y = curs->y - 1 < 0 ? curs->y : curs->y - 1;
            goto move;
        case KEY_RIGHT:
        case 'l':
            curs->x = curs->x + 1 >= LINE_LEN ? curs->x : curs->x + 1;
            goto move;
        move:
            move_cursor(curs, opts->small_mode);
            break;
        // Save file and handle errors
        case 's':
            if (!savestate(opts->filename, sudoku))
                sprintf(spec->statusbar, "Error: '%s'\n", strerror(errno));
            else
                sprintf(spec->statusbar, "%s", "Saved");

            draw(spec);

            break;
        // Check for errors and write result to statusbar
        case 'c':
        {
            char combined_solution[SUDOKU_LEN];
            for (int i = 0; i < SUDOKU_LEN; i++) {
                combined_solution[i] =
                    sudoku->sudoku[i] == '0' ? sudoku->user[i] : sudoku->sudoku[i];
            }

            if (check_validity(combined_solution))
                sprintf(spec->statusbar, "%s", "Valid");
            else
                sprintf(spec->statusbar, "%s", "Invalid or not filled out");

            draw(spec);
            break;
        }
        // Fill out sudoku; ask for confirmation first
        case 'd':
            if (!status_bar_confirmation(spec))
                break;

            char combined_solution[SUDOKU_LEN];
            for (int i = 0; i < SUDOKU_LEN; i++) {
                combined_solution[i] =
                    sudoku->sudoku[i] == '0' ? sudoku->user[i] : sudoku->sudoku[i];
            }

            solve(combined_solution, false);
            memcpy(sudoku->user, combined_solution, SUDOKU_LEN);

            draw(spec);
            break;
        // Enter edit mode
        case 'e':
            if (opts->small_mode)
                break;
            editing_notes = !editing_notes;
            char *mode = editing_notes ? "Note" : "Normal";
            sprintf(spec->statusbar, "%s %s", mode, "Mode");

            draw(spec);
            break;
        case 'g':
            input_go_to(spec);
            break;
        case 'v':
        {
            sprintf(spec->statusbar, "%s", "Highlight:");
            draw(spec);

            spec->highlight = getch();
            if (spec->highlight < '1' || spec->highlight > '9') {
                sprintf(spec->statusbar, "%s", "Cancelled");
            } else {
                sprintf(spec->statusbar, "%s%c", "Highlight: ", spec->highlight);
            }

            draw(spec);
            break;
        }
        // Exit; ask for confirmation
        case 'q':
            if (!status_bar_confirmation(spec))
                break;

            quit = true;
            break;
        // Input numbers into the user sudoku field
        default:
            // Check if the key is a number (not zero) in aasci chars or 'x' and
            // if the cursor is not an a field filled by the puzzle

            // Check if the field is empty in the puzzle
            if (sudoku->sudoku[curs->y * LINE_LEN + curs->x] == '0') {
                // Toggle the note fields (if in note mode)
                if (editing_notes) {
                    if (key_press >= '1' && key_press <= '9') {
                        // Access cursor location in array and add key_press for
                        // appropriate number
                        int *target = &sudoku->notes[((curs->y * LINE_LEN * LINE_LEN) +
                                                     (curs->x * LINE_LEN)) +
                                                     (key_press - '1')];
                        *target = !*target;
                        draw(spec);
                    }
                    // Check for numbers and place the number in user_nums
                } else if (key_press >= '1' && key_press <= '9' &&
                           sudoku->user[curs->y * LINE_LEN + curs->x] !=
                               key_press) {
                    sudoku->user[curs->y * LINE_LEN + curs->x] = key_press;
                    // Clear notes off of target cell
                    for (int i = 0; i < LINE_LEN; i++) {
                        int *target = &sudoku->notes[((curs->y * LINE_LEN * LINE_LEN) +
                                                     (curs->x * LINE_LEN)) + i];
                        *target = 0;
                    }
                    draw(spec);
                }
                // Check for x and clear the number (same as pressing space in
                // the above conditional)
                else if ((key_press == 'x' || key_press == '0') &&
                         sudoku->user[curs->y * LINE_LEN + curs->x] != '0') {
                    sudoku->user[curs->y * LINE_LEN + curs->x] = '0';
                    draw(spec);
                }
            }
            break;
        }
    }

    spec->highlight = 0;
}

int main(int argc, char **argv)
{
    struct TSOpts opts = {
        .gen_visual = false,
        .own_sudoku = false,
        .attempts = ATTEMPTS_DEFAULT,
        .dir = NULL,
        .from_file = false,
        .ask_confirmation = true,
        .small_mode = false,
    };

    // Handle command line input with getopt
    int flag;
    while ((flag = getopt(argc, argv, "hsvfecd:n:")) != -1) {
        switch (flag) {
        case 'h':
            printf("term-sudoku Copyright (C) 2021 eyeofcthulhu\n\n"
                   "usage: term-sudoku [-hsvfec] [-d DIR] [-n NUMBER]\n\n"
                   "flags:\n"
                   "-h: display this information\n"
                   "-s: small mode (disables noting numbers)\n"
                   "-v: generate the sudoku visually\n"
                   "-f: list save games and use a selected file as the sudoku\n"
                   "-e: enter your own sudoku\n"
                   "-c: do not ask for confirmation when trying to exit, "
                   "solve, etc.\n"
                   "-d: DIR: specify directory where save files are and should "
                   "be saved\n"
                   "-n: NUMBER: numbers to try and remove (default: %d)\n\n"
                   "controls:\n"
                   "%s",
                   ATTEMPTS_DEFAULT, controls_default);
            return 0;
        case 'v':
            opts.gen_visual = true;
            break;
        case 'e':
            opts.own_sudoku = true;
            break;
        case 'n':
            opts.attempts = strtol(optarg, NULL, 10);
            if (opts.attempts <= 0)
                opts.attempts = ATTEMPTS_DEFAULT;
            break;
        case 'd':
            opts.dir = optarg;
            break;
        case 'f':
            opts.from_file = true;
            break;
        case 'c':
            opts.ask_confirmation = false;
            break;
        case 's':
            opts.small_mode = true;
            break;
        case '?':
        default:
            return 1;
        }
    }

    // on interrupt and segfault (Ctrl+c) exit (call finish (exit ncurses))
    signal(SIGINT, finish);
    signal(SIGSEGV, finish);

    // Seed random
    srand(time(NULL));

    // Set dir as $HOME/.local/share
    if (opts.dir == NULL) {
        char *target_dir, *home_dir;

        // Get user home directory
        struct passwd *pw = getpwuid(getuid());
        home_dir = pw->pw_dir;

        // target is: $HOME/.local/share/term-sudoku
        target_dir =
            malloc((strlen(home_dir) + strlen(appsharepath) + 2) * sizeof(char));

        sprintf(target_dir, "%s/%s", home_dir, sharepath);

        // Create (if not already created) the term-sudoku directory in the
        // .local/share directory
        struct stat st;
        if (stat(target_dir, &st) != -1) {
            sprintf(target_dir, "%s/%s", home_dir, appsharepath);

            // ~/.local/share exists but ~/.local/share/term-sudoku doesn't
            if (stat(target_dir, &st) == -1) {
                if ((mkdir(target_dir, 0777)) == -1) {
                    finish_with_errno("Creating directory %s", target_dir);
                }
            }
        } else {
            // Fallback to current working directory, since ~/.local/share doesn't exist
            target_dir = ".";
        }
        opts.dir = target_dir;
    }

    struct SudokuSpec sudoku;
    struct Cursor cursor;

    struct TSStruct spec = {
        .opts = &opts,
        .sudoku = &sudoku,
        .cursor = &cursor,
        .highlight = 0,
        .controls = (char *)controls_default,
    };
    memset(spec.statusbar, '\0', sizeof(spec.statusbar));

    spec.opts = &opts;
    spec.sudoku = &sudoku;
    spec.cursor = &cursor;

    // ncurses intializer functions
    init_ncurses();

    if (opts.gen_visual)
        init_visual_generator(&spec);

    // List files and open selected file or create a new file or enter your own
    // sudoku
    if (opts.from_file) {
        // Run fileview()(which runs new_sudoku(), fileview() and mainloop()
        // depending on input) until it returns false
        while (fileview(&spec))
            ;
    }
    // User enters a new sudoku and edits it in mainloop() if successful
    else if (opts.own_sudoku) {
        if (own_sudoku_view(&spec))
            mainloop(&spec);
        // Not own_sudoku nor from_file: generate new sudoku
    } else {
        new_sudoku(&spec);
        mainloop(&spec);
    }
    finish(0);
}
