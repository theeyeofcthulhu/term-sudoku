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
#include "util.h"
#include "sudoku.h"

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>
#include <sys/stat.h>
#include <curses.h>
#include <errno.h>

void new_sudoku();
void input_go_to();
bool own_sudoku_view();
bool fileview();
void mainloop();

char* controls_default = "move - h, j, k and l or arrow keys\n"
					"1-9 - insert numbers\n"
					"x or 0 - delete numbers\n"
					"save - s\n"
					"check for errors - c\n"
					"solve sudoku - d\n"
					"notetaking mode - e\n"
					"go to position - g\n"
					"highlight number - v\n"
					"quit - q\n";
char* controls;

bool editing_notes = false;

char* filename;
char* sharepath = ".local/share/term-sudoku";
char* target_dir;

TSOpts opts;

// Generate a new sudoku for the user to solve
void new_sudoku(){
	gen_file_name();

	// Clear sudoku arrays
	memset(sudoku_str, '0', SUDOKU_LEN);
	memset(user_nums, '0', SUDOKU_LEN);
	memset(notes, 0, (sizeof(notes) / sizeof(notes[0]) * sizeof(int)));

	// Generate the sudoku
	generate_sudoku(sudoku_str);

	sprintf(statusbar, "%s", "Sudoku generated");
}

// Ask for position (getch()) and go there
void input_go_to(){
	int move_to[2] = {0, 0};

	sprintf(statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
	draw();

	for(int i = 0; i < 2; i++){
		char c_pos = getch();
		move_to[i] = strtol(&c_pos, NULL, 10);
		if(move_to[i] <= 0 || move_to[i] > 9){
			sprintf(statusbar, "%s", "Cancelled");
			draw();
			return;
		}

		sprintf(statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
		draw();
	}
	move_cursor_to(move_to[0] - 1, move_to[1] - 1);
}

bool own_sudoku_view(){
	cursor.x = cursor.y = 0;

	gen_file_name();

	// Clear sudoku arrays
	memset(sudoku_str, '0', SUDOKU_LEN);
	memset(user_nums, '0', SUDOKU_LEN);
	memset(notes, 0, (sizeof(notes) / sizeof(notes[0]) * sizeof(int)));

	sprintf(statusbar, "%s", "Enter your sudoku");
	// Controls displayed only in this view
	char* custom_sudoku_controls = "move - h, j, k and l or arrow keys\n"
					"1-9 - insert numbers\n"
					"x or 0 - delete numbers\n"
					"done - d\n"
					"go to position - g\n"
					"quit - q\n";
	controls = custom_sudoku_controls;
	// Draw with new controls
	draw();
	bool done = false;
	bool quit = false;

	// Loop for entering own sudoku
	while(!done && !quit){
		int key_press = getch();
		// Move on vim keys and bind to field size
		switch(key_press){
		case KEY_LEFT:
		case 'h':
			cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
			move_cursor();
			break;
		case KEY_DOWN:
		case 'j':
			cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
			move_cursor();
			break;
		case KEY_UP:
		case 'k':
			cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
			move_cursor();
			break;
		case KEY_RIGHT:
		case 'l':
			cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
			move_cursor();
			break;
		case 'd':
			if(!status_bar_confirmation()) break;

			sprintf(statusbar, "Sudoku entered");
			done = true;
			break;
		case 'g':
			input_go_to();
			break;
		case 'q':
			if(!status_bar_confirmation()) break;

			quit = true;
			break;
		// Input numbers into the user sudoku field
		default:
			// Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

			// check for numbers
			if(key_press >= '1' && key_press <= '9' && sudoku_str[cursor.y * LINE_LEN + cursor.x] != key_press){
				sudoku_str[cursor.y * LINE_LEN + cursor.x] = key_press;
				draw();
			}
			// check for x
			else if((key_press == 'x' || key_press == '0') && sudoku_str[cursor.y * LINE_LEN + cursor.x] != '0'){
				sudoku_str[cursor.y * LINE_LEN + cursor.x] = '0';
				draw();
			}
			break;
		}
	}
	// Reset controls
	controls = controls_default;
	if(quit)
		return false;
	return true;
}

bool fileview(){
	// Array of strings (in this case: directories)
	char* items[STR_LEN];
	// Holds the size of 'items'
	int iterator = 0;

	// Load files in directory into items
	listfiles(target_dir, items, &iterator);

	curs_set(0);

	bool chosen = false;
	bool new_file = false;
	bool own = false;
	int position = 0;

	char* file_view_controls = "Choose a savegame - move - j and k, d - delete, confirm - y, new file - n, own sudoku - o, quit - q";

	// Choose file by moving cursor
	while(!chosen && !new_file && !own){

		erase();

		mvprintw(0, 0, file_view_controls);

		for(int j = 0; j < iterator; j++)
			mvprintw(j + 2, 0, "  %s\n", items[j]);

		// Asteriks as cursor for file selection
		mvaddch(position + 2, 0, '*');

		int key_press = getch();

		// Move on vim keys and bind to item size later
		switch(key_press){
			case KEY_DOWN:
			case 'j':
				position += 1;
				break;
			case KEY_UP:
			case 'k':
				position -= 1;
				break;
			case 'y':
				if(iterator > 0)
					chosen = true;
				break;
			case 'o':
				own = true;
				break;
			// Delete selected file and re-read files
			case 'd':
			{
				char* temp_file = malloc((strlen(target_dir) + strlen(items[position]) + 2) * sizeof(char));
				sprintf(temp_file, "%s/%s", target_dir, items[position]);
				if(remove(temp_file) == -1)
					finish_with_err_msg("Error: '%s' when trying to remove '%s'\n", strerror(errno), temp_file);
				for(int j = 0; j < iterator; j++)
					free(items[j]);
				listfiles(target_dir, items, &iterator);
				free(temp_file);
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

		if(iterator != 0){
			// Wrap position according to list size
			if(position >= iterator)
				position = 0;
			else if (position < 0)
				position = iterator - 1;
		// Keep position 0 if no files are available
		}else
			position = 0;
	}

	curs_set(1);

	// Reading the file
	if(chosen){
		sprintf(filename, "%s/%s", target_dir, items[position]);

		for(int j = 0; j < iterator; j++)
			free(items[j]);

		// Read Sudoku from given file
		FILE* input_file = fopen(filename, "r");
		if(input_file == NULL)
			finish_with_err_msg("Error: '%s' when trying to access file '%s'\n", strerror(errno), filename);

		// Scan in the first two lines which are the puzzle and the entered user_nums
		fscanf(input_file, "%s\n%s\n", sudoku_str, user_nums);
		// Read in notes into the array
		for(int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
			fscanf(input_file, "%1d", &notes[i]);
		fclose(input_file);

		sprintf(statusbar, "%s", "File opened");

		mainloop();
	}else if(own){
		if(own_sudoku_view())
			mainloop();
	}
	else if(new_file){
		new_sudoku();
		mainloop();
	}

	return true;
}

/*
** Draws the sudoku and processes input relating to modifying the sudoku, changing something about the rendering or moving the cursor
*/
void mainloop(){
	cursor.x = cursor.y = 0;

	draw();
	// Main loop: wait for keypress, then process it
	while(true){
		int key_press = getch();
		switch(key_press){
			// Move on vim keys and bind to field size
			case KEY_LEFT:
			case 'h':
				cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
				move_cursor();
				break;
			case KEY_DOWN:
			case 'j':
				cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
				move_cursor();
				break;
			case KEY_UP:
			case 'k':
				cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
				move_cursor();
				break;
			case KEY_RIGHT:
			case 'l':
				cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
				move_cursor();
				break;
			// Save file and handle errors
			case 's':
				if(!savestate())
					sprintf(statusbar, "Error: '%s'\n", strerror(errno));
				else
					sprintf(statusbar, "%s", "Saved");

				draw();

				break;
			// Check for errors and write result to statusbar
			case 'c':
			{
				char* combined_solution = malloc((SUDOKU_LEN + 1) * sizeof(char));
				for(int i = 0; i < SUDOKU_LEN; i++)
					combined_solution[i] = sudoku_str[i] == '0' ? user_nums[i] : sudoku_str[i];

				if(check_validity(combined_solution))
					sprintf(statusbar, "%s", "Valid");
				else
					sprintf(statusbar, "%s", "Invalid or not filled out");

				draw();

				free(combined_solution);
				break;
			}
			// Fill out sudoku; ask for confirmation first
			case 'd':
				if(!status_bar_confirmation()) break;

				solve_user_nums();
				draw();
				break;
			// Enter edit mode
			case 'e':
				if(opts.small_mode)
					break;
				editing_notes = !editing_notes;
				char* mode = editing_notes ? "Note" : "Normal";
				sprintf(statusbar, "%s %s", mode, "Mode");

				draw();
				break;
			case 'g':
				input_go_to();
				break;
			case 'v':
			{
				sprintf(statusbar, "%s", "Highlight:");
				draw();

				highlight = getch();
				if(highlight < '1' || highlight > '9'){
					sprintf(statusbar, "%s", "Cancelled");
				}else
					sprintf(statusbar, "%s%c", "Highlight: ", highlight);

				draw();
				break;
			}
			// Exit; ask for confirmation
			case 'q':
				if(!status_bar_confirmation()) break;

				highlight = 0;
				return;
			// Input numbers into the user sudoku field
			default:
				// Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

				// Check if the field is empty in the puzzle
				if(sudoku_str[cursor.y * LINE_LEN + cursor.x] == '0'){
					// Toggle the note fields (if in note mode)
					if(editing_notes){
						if(key_press >= '1' && key_press <= '9'){
							// Access cursor location in array and add key_press for appropriate number
							int* target = &notes[((cursor.y * LINE_LEN * LINE_LEN) + (cursor.x * LINE_LEN)) + (key_press - '1')];
							*target = !*target;
							draw();
						}
					// Check for numbers and place the number in user_nums
					}else if(key_press >= '1' && key_press <= '9' && user_nums[cursor.y * LINE_LEN + cursor.x] != key_press){
						user_nums[cursor.y * LINE_LEN + cursor.x] = key_press;
						// Clear notes off of target cell
						for (int i = 0; i < LINE_LEN; i++) {
							int* target = &notes[((cursor.y * LINE_LEN * LINE_LEN) + (cursor.x * LINE_LEN)) + i];
							*target = 0;
						}
						draw();
					}
					// Check for x and clear the number (same as pressing space in the above conditional)
					else if((key_press == 'x' || key_press == '0') && user_nums[cursor.y * LINE_LEN + cursor.x] != '0'){
						user_nums[cursor.y * LINE_LEN + cursor.x] = '0';
						draw();
					}
				}
				break;
		}
	}
}

int main(int argc, char **argv){
	controls = controls_default;

	opts = (TSOpts){
		.gen_visual = false,
		.own_sudoku = false,
		.attempts = ATTEMPTS_DEFAULT,
		.custom_dir = NULL,
		.from_file = false,
		.ask_confirmation = true,
		.small_mode = false,
	};

	// Handle command line input with getopt
	int flag;
	while ((flag = getopt(argc, argv, "hsvfecd:n:")) != -1){
		switch (flag){
		case 'h':
			printf(	"term-sudoku Copyright (C) 2021 eyeofcthulhu\n\n"
					"usage: term-sudoku [-hsvfec] [-d DIR] [-n NUMBER]\n\n"
					"flags:\n"
					"-h: display this information\n"
					"-s: small mode (disables noting numbers)\n"
					"-v: generate the sudoku visually\n"
					"-f: list save games and use a selected file as the sudoku\n"
					"-e: enter your own sudoku\n"
					"-c: do not ask for confirmation when trying to exit, solve, etc.\n"
					"-d: DIR: specify directory where save files are and should be saved\n"
					"-n: NUMBER: numbers to try and remove (default: %d)\n\n"
					"controls:\n"
					"%s", ATTEMPTS_DEFAULT, controls);
			return 0;
		case 'v':
			opts.gen_visual = true;
			break;
		case 'e':
			opts.own_sudoku = true;
			break;
		case 'n':
			opts.attempts = strtol(optarg, NULL, 10);
			if(opts.attempts <= 0 || opts.attempts >= SUDOKU_LEN)
				opts.attempts = ATTEMPTS_DEFAULT;
			break;
		case 'd':
			opts.custom_dir = malloc((strlen(optarg) + 1) * sizeof(char));
			strcpy(opts.custom_dir, optarg);
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
	if(opts.custom_dir == NULL){
		char* home_dir;
		// Get user home directory
		struct passwd *pw = getpwuid(getuid());
		home_dir = pw->pw_dir;
		// target is: $HOME/.local/share/term-sudoku
		target_dir = malloc((strlen(home_dir) + strlen(sharepath) + 2) * sizeof(char));
		sprintf(target_dir, "%s/%s", home_dir, sharepath);
		// Create (if not already created) the term-sudoku directory in the .local/share directory
		struct stat st = { 0 };
		if(stat(target_dir, &st) == -1){
			if((mkdir(target_dir, 0777)) == -1)
				finish_with_err_msg("Error: '%s' when trying to create directory '%s'\n", strerror(errno), target_dir);
		}
	// Use a custom directory specified in '-d'
	}else
		target_dir = opts.custom_dir;

	// Allocate strings, fill with zeros and null-terminate
	sudoku_str = malloc((SUDOKU_LEN + 1) * sizeof(char));
	user_nums = malloc((SUDOKU_LEN + 1) * sizeof(char));

	memset(sudoku_str, '0', SUDOKU_LEN);
	sudoku_str[SUDOKU_LEN] = '\0';
	memset(user_nums, '0', SUDOKU_LEN);
	user_nums[SUDOKU_LEN] = '\0';

	// String for the statusbar and filename
	statusbar = malloc(STR_LEN * sizeof(char));
	filename = malloc(STR_LEN * sizeof(char));

	// ncurses intializer functions
	init_ncurses();

	// List files and open selected file or create a new file or enter your own sudoku
	if(opts.from_file){
		// Run fileview()(which runs new_sudoku(), fileview() and mainloop() depending on input)
		// until it returns false
		while(fileview());
		finish(0);
	}
	// User enters a new sudoku and edits it in mainloop() if successful
	else if(opts.own_sudoku){
		if(own_sudoku_view())
			mainloop();
		finish(0);
	// Not own_sudoku nor from_file: generate new sudoku
	}else{
		new_sudoku();
		mainloop();
		finish(0);
	}
}
