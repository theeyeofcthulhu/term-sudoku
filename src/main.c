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

char* controls_default = "move - h, j, k and l\n"
					"1-9 - insert numbers\n"
					"x or 0 - delete numbers\n"
					"save - s\n"
					"check for errors - c\n"
					"solve sudoku - d\n"
					"notetaking mode - e\n"
					"go to position - g\n"
					"quit - q\n";
char* controls;

bool editing_notes = false;

bool from_file = false;
bool own_sudoku = false;

char* custom_dir;

char* filename;
char* sharepath = ".local/share/term-sudoku";
char* home_dir;
char* target_dir;

int main(int argc, char **argv){
	controls = controls_default;

	// Handle command line input with getopt
	int flag;
	while ((flag = getopt(argc, argv, "hsvfecd:n:")) != -1){
		switch (flag){
		case 'h':
			printf(	"term-sudoku Copyright (C) 2021 eyeofcthulhu\n\n"
					"usage: term-sudoku [-hsvfe] [-d DIR] [-n NUMBER]\n\n"
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
			sudoku_gen_visual = true;
			break;
		case 'e':
			own_sudoku = true;
			break;
		case 'n':
			sudoku_attempts = strtol(optarg, NULL, 10);
			if(sudoku_attempts <= 0 || sudoku_attempts >= SUDOKU_LEN)
				sudoku_attempts = ATTEMPTS_DEFAULT;
			break;
		case 'd':
			if(strlen(optarg) >= STR_LEN)
				finish_with_err_msg("Directory name too long!\n");
			custom_dir = malloc(STR_LEN * sizeof(char));
			strcpy(custom_dir, optarg);
			break;
		case 'f':
			from_file = true;
			break;
		case 'c':
			util_ask_confirmation = false;
			break;
		case 's':
			render_small_mode = true;
			break;
		case '?':
			return 1;
		default:
			return 1;
		}
	}

	// Can't open from file and have user enter their own sudoku
	if(from_file && own_sudoku)
		finish_with_err_msg("Mutually exclusive flags given!\n");

	// Initialize time and seed random
	time_t t = time(NULL);
	srand((unsigned) time(&t));

	// Set dir as $HOME/.local/share
	if(custom_dir == NULL){
		// Get user home directory
		struct passwd *pw = getpwuid(getuid());
		home_dir = pw->pw_dir;
		// target is: $HOME/.local/share/term-sudoku
		target_dir = malloc(STR_LEN * sizeof(char));
		sprintf(target_dir, "%s/%s", home_dir, sharepath);
		// Create (if not already created) the term-sudoku directory in the .local/share directory
		struct stat st = { 0 };
		if(stat(target_dir, &st) == -1)
			mkdir(target_dir, 0777);
	// Use a custom directory specified in '-d'
	}else
		target_dir = custom_dir;

	// Allocate strings, fill with zeros and null-terminate
	init_sudoku_strings();

	// String for the statusbar and filename
	statusbar = malloc(STR_LEN * sizeof(char));
	filename = malloc(STR_LEN * sizeof(char));

	// ncurses intializer functions
	init_ncurses();

	// initialize cursor struct, responsible for storing x and y coordinations for the cursor
	cursor.x = 0;
	cursor.y = 0;

	// List files and open selected file
	if(from_file){
		// Array of strings (in this case: directories)
		char* items[STR_LEN];
		// Holds the size of 'items'
		int iterator = 0;

		// Load files in directory into items
		listfiles(target_dir, items, &iterator);

		curs_set(0);

		bool chosen = false;
		bool new_file = false;
		int position = 0;

		char* temp_file;

		// Choose file by moving cursor
		while(!chosen && !new_file){

			erase();

			mvprintw(0, 0, "Choose a savegame - move - j and k, d - delete, confirm - y, new file - n, quit - q");

			for(int j = 0; j < iterator; j++)
				mvprintw(j + 2, 0, "  %s\n", items[j]);

			// Asteriks as cursor for file selection
			mvaddch(position + 2, 0, '*');

			char key_press = getch();

			// Move on vim keys and bind to item size later
			switch(key_press){
				case 'j':
					position += 1;
					break;
				case 'k':
					position -= 1;
					break;
				case 'y':
					chosen = true;
					break;
				// Delete selected file and re-read files
				case 'd':
					temp_file = malloc(STR_LEN * sizeof(char));
					sprintf(temp_file, "%s/%s", target_dir, items[position]);
					remove(temp_file);
					for(int j = 0; j < iterator; j++)
						free(items[j]);
					listfiles(target_dir, items, &iterator);
					free(temp_file);
					break;
				case 'q':
					finish(0);
				// Create a new file instead of reading one
				case 'n':
					new_sudoku(filename, target_dir, t);
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
				finish_with_err_msg("Error accessing file '%s'\n", filename);

			// Scan in the first two lines which are the puzzle and the entered user_nums
			fscanf(input_file, "%s\n%s\n", sudoku_str, user_nums);
			// Read in notes into the array
			for(int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
				fscanf(input_file, "%1d", &notes[i]);
			fclose(input_file);

			sprintf(statusbar, "%s", "File opened");
		}
	// User enters a new sudoku
	}else if(own_sudoku){
		sprintf(statusbar, "%s", "Enter your sudoku");
		// Controls displayed only in this view
		char* custom_sudoku_controls = "move - h, j, k and l\n"
						"1-9 - insert numbers\n"
						"x or 0 - delete numbers\n"
						"done - d\n"
						"quit - q\n";
		controls = custom_sudoku_controls;
		// Draw with new controls
		draw();
		bool done = false;

		// Loop for entering own sudoku
		while(!done){
			char key_press = getch();
			// Move on vim keys and bind to field size
			switch(key_press){
			case 'h':
				cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
				move_cursor();
				break;
			case 'j':
				cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
				move_cursor();
				break;
			case 'k':
				cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
				move_cursor();
				break;
			case 'l':
				cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
				move_cursor();
				break;
			case 'd':
				if(!status_bar_confirmation()) break;

				sprintf(statusbar, "Sudoku entered");
				done = true;
				break;
			case 'q':
				if(!status_bar_confirmation()) break;

				finish(0);
			// Input numbers into the user sudoku field
			default:
				// Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

				// check for numbers
				if(key_press >= 0x30 && key_press <= 0x39 && sudoku_str[cursor.y * LINE_LEN + cursor.x] != key_press){
					sudoku_str[cursor.y * LINE_LEN + cursor.x] = key_press;
					draw();
				}
				// check for x
				else if(key_press == 'x' && sudoku_str[cursor.y * LINE_LEN + cursor.x] != '0'){
					sudoku_str[cursor.y * LINE_LEN + cursor.x] = '0';
					draw();
				}
				break;
			}
		}
		// Reset controls
		controls = controls_default;
	// Not own_sudoku nor from_file: generate new sudoku
	}else
		new_sudoku(filename, target_dir, t);

	draw();

	// Main loop: wait for keypress, then process it
	while(true){
		char key_press = getch();
		switch(key_press){
			// Move on vim keys and bind to field size
			//
			case 'h':
				cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
				move_cursor();
				break;
			case 'j':
				cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
				move_cursor();
				break;
			case 'k':
				cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
				move_cursor();
				break;
			case 'l':
				cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
				move_cursor();
				break;
			// Save file and handle errors
			case 's':
				if(!savestate(filename))
					finish_with_err_msg("Error saving file '%s'\n", filename);
				else
					sprintf(statusbar, "%s", "Saved");

				draw();

				break;
			// Check for errors and write result to statusbar
			case 'c':
			{
				char* combined_solution = malloc((SUDOKU_LEN) * sizeof(char));
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
				if(render_small_mode)
					break;
				editing_notes = !editing_notes;
				char* mode = editing_notes == 1 ? "Note" : "Normal";
				sprintf(statusbar, "%s %s", mode, "Mode");

				draw();
				break;
			case 'g':
			{
				char* statusbar_backup = malloc(STR_LEN * sizeof(char));
				strcpy(statusbar_backup, statusbar);

				int move_to[2] = {0, 0};

				sprintf(statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
				draw();

				bool abort = false;

				for(int i = 0; i < 2; i++){
					char c_pos = getch();
					move_to[i] = strtol(&c_pos, NULL, 10);
					if(move_to[i] <= 0 || move_to[i] > 9){
						sprintf(statusbar, "%s", statusbar_backup);
						free(statusbar_backup);
						draw();
						abort = true;
						break;
					}

					sprintf(statusbar, "Move to: %d, %d", move_to[0], move_to[1]);
					draw();
				}

				if(abort) break;

				move_cursor_to(move_to[1] - 1, move_to[0] - 1);

				free(statusbar_backup);
				break;
			}
			// Exit; ask for confirmation
			case 'q':
				if(!status_bar_confirmation()) break;

				finish(0);
			// Input numbers into the user sudoku field
			default:
				// Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

				// Check if the field is empty in the puzzle
				if(sudoku_str[cursor.y * LINE_LEN + cursor.x] == '0'){
					// Toggle the note fields (if in note mode)
					if(editing_notes){
						if(key_press > 0x30 && key_press <= 0x39){
							// Access cursor location in array and add key_press for appropriate number
							int* target = &notes[((cursor.y * LINE_LEN * LINE_LEN) + (cursor.x * LINE_LEN)) + (key_press - 0x31)];
							*target = !*target;
							draw();
						}
					// Check for numbers and place the number in user_nums
					}else if(key_press >= 0x30 && key_press <= 0x39 && user_nums[cursor.y * LINE_LEN + cursor.x] != key_press){
						user_nums[cursor.y * LINE_LEN + cursor.x] = key_press;
						// Clear notes off of target cell
						for (int i = 0; i < LINE_LEN; i++) {
							int* target = &notes[((cursor.y * LINE_LEN * LINE_LEN) + (cursor.x * LINE_LEN)) + i];
							*target = 0;
						}
						draw();
					}
					// Check for x and clear the number (same as pressing space in the above conditional)
					else if(key_press == 'x' && user_nums[cursor.y * LINE_LEN + cursor.x] != '0'){
						user_nums[cursor.y * LINE_LEN + cursor.x] = '0';
						draw();
					}
				}
				break;
		}
	}
}
