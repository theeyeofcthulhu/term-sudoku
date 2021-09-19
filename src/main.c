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

char* user_nums;
char* sudoku_str;
struct cursor cursor;
char* statusbar;
char* controls = "move - h, j, k and l\n"
				 "1-9 - insert numbers\n"
				 "x or 0 - delete numbers\n"
				 "save - s\n"
				 "check for errors - c\n"
				 "solve sudoku - d\n"
				 "notetaking mode - e\n"
				 "quit - q\n";
int editing_notes = 0;
int* notes;

int attempts = ATTEMPTS_DEFAULT;
int gen_visual = 0;
int from_file = 0;
int small_mode = 0;

char* filename;
char* sharepath = ".local/share/term-sudoku";
char* home_dir;
char* target_dir;

int main(int argc, char **argv){
	int flag;
	while ((flag = getopt (argc, argv, "hsvfn:")) != -1){
		switch (flag){
		case 'h':
			printf(	"term-sudoku Copyright (C) 2021 eyeofcthulhu\n\n"
					"usage: term-sudoku [-hsvf] [-n NUMBER]\n\n"
					"flags:\n"
					"-h: display this information\n"
					"-s: small mode (disables noting numbers)\n"
					"-v: generate the sudoku visually\n"
					"-f: list save games and use a selected file as the sudoku\n"
					"-n: NUMBER: numbers to try and remove (default: %d)\n\n"
					"controls:\n"
					"%s", ATTEMPTS_DEFAULT, controls);
			return 0;
		case 'v':
			gen_visual = 1;
			break;
		case 'f':
			from_file = 1;
			break;
		case 'n':
			attempts = strtol(optarg, NULL, 10);	
			if(attempts == 0 || attempts >= SUDOKU_LEN)
				attempts = ATTEMPTS_DEFAULT;
			break;
		case 's':
			small_mode = 1;
			break;
		case '?':
			return 1;
		default:
			return 1;
		}
	}

	//--- USER INIT LOGIC ---
	
	// Initialize flag values in other files
	sudoku_gen_visual = gen_visual;
	sudoku_attempts = attempts;
	render_small_mode = small_mode;

	//Initialize time and seed random
	time_t t = time(NULL);
	srand((unsigned) time(&t));

	// Get user home directory
	struct passwd *pw = getpwuid(getuid());
	home_dir = pw->pw_dir;

	// target is: $HOME/.local/share/term-sudoku
	target_dir = malloc(STR_LEN * sizeof(char));
	sprintf(target_dir, "%s/%s", home_dir, sharepath);

	// Create (if not already created) the term-sudoku directory in the .local/share directory
	struct stat st = { 0 };
	if(stat(target_dir, &st) == -1){
		mkdir(target_dir, 0777);
	}

	// Allocate strings, fill with zeros and null-terminate

	// Array for sudoku
	sudoku_str = malloc((SUDOKU_LEN + 1) * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; sudoku_str[i++] = '0');
	sudoku_str[SUDOKU_LEN] = '\0';

	// Array for numbers the user enters
	user_nums = malloc((SUDOKU_LEN + 1) * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; user_nums[i++] = '0');
	user_nums[SUDOKU_LEN] = '\0';

	// Array for notetaking
	notes = malloc((SUDOKU_LEN * LINE_LEN + 1) * sizeof(int));
	for(int i = 0; i < SUDOKU_LEN * LINE_LEN; notes[i++] = 0);
	notes[SUDOKU_LEN * LINE_LEN] = '\0';

	// String for the statusbar
	statusbar = malloc(STR_LEN * sizeof(char));

	filename = malloc(STR_LEN * sizeof(char));

	// List files and open selected file
	if(from_file){
		init_ncurses();

		// Array of strings
		char* items[STR_LEN];
		int iterator = 0;

		// Load files in directory into items
		listfiles(target_dir, items, &iterator);

		curs_set(0);

		int chosen = 0;
		int position = 0;

		char* temp_file = malloc(STR_LEN * sizeof(char));

		// Choose file by moving cursor
		while(!chosen){

			erase();

			mvprintw(0, 0, "Choose a savegame - move - j and k, d - delete, confirm - y");

			for(int j = 0; j < iterator; j++)
				mvprintw(j + 2, 0, "  %s\n", items[j]);

			mvaddch(position + 2, 0, '*');

			char key_press = getch();

			//Move on vim keys and bind to field size
			switch(key_press){
				case 'j':
					position += 1;
					break;
				case 'k':
					position -= 1;
					break;
				case 'y':
					chosen = 1;
					break;
				// Delete selected file and re-read files
				case 'd':
					sprintf(temp_file, "%s/%s", target_dir, items[position]);
					remove(temp_file);
					for(int j = 0; j < iterator; j++)
						free(items[j]);
					listfiles(target_dir, items, &iterator);
					break;
				default:
					break;
			}

			if(position >= iterator)
				position = 0;
			else if (position < 0)
				position = iterator - 1;
		}

		free(temp_file);

		curs_set(1);

		sprintf(filename, "%s/%s", target_dir, items[position]);

		//Read Sudoku from given file
		FILE* input_file = fopen(filename, "r");
		if(input_file == NULL)
			finish_with_err_msg("Error accessing file\n");

		fscanf(input_file, "%s\n%s\n", sudoku_str, user_nums);
		for(int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
			fscanf(input_file, "%1d", &notes[i]);
		fclose(input_file);

		sudoku_str[SUDOKU_LEN] = '\0';

		sprintf(statusbar, "%s", "File opened");
	// Generate a new sudoku
	}else{
		// localtime struct for file name
		struct tm tm = *localtime(&t);
		char* filename_no_dir = malloc(STR_LEN * sizeof(char));
		// Generate file name with the current time
		sprintf(filename_no_dir, "%4d%02d%02d%02d%02d%02d%s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ".sudoku");

		sprintf(filename, "%s/%s", target_dir, filename_no_dir);

		free(filename_no_dir);

		init_ncurses();

		if(gen_visual)
			curs_set(0);
		generate_sudoku(sudoku_str);
		if(gen_visual)
			curs_set(1);

		sprintf(statusbar, "%s", "Sudoku generated");
	}

	cursor.x = 0;
	cursor.y = 0;

	draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);

	//Main loop: wait for keypress, then process it
	while(true){
		char key_press = getch();

		char* combined_solution;
		char* statusbar_backup;

		//Move on vim keys and bind to field size
		switch(key_press){
			case 'h':
				cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
				move_cursor(cursor);
				break;
			case 'j':
				cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
				move_cursor(cursor);
				break;
			case 'k':
				cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
				move_cursor(cursor);
				break;
			case 'l':
				cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
				move_cursor(cursor);
				break;
			case 's':
				//Save file and handle errors
				if(!savestate())
					finish_with_err_msg("Error accessing file\n");
				else
					sprintf(statusbar, "%s", "Saved");

				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);

				break;
			case 'c':
				//Check for errors (writes to statusbar directly)

				combined_solution = malloc((SUDOKU_LEN) * sizeof(char));
				for(int i = 0; i < SUDOKU_LEN; i++)
					combined_solution[i] = sudoku_str[i] == '0' ? user_nums[i] : sudoku_str[i];

				if(check_validity(combined_solution))
					sprintf(statusbar, "%s", "Valid");
				else
					sprintf(statusbar, "%s", "Invalid or not filled out");

				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);

				free(combined_solution);
				break;
			//Fill out sudoku; ask for confirmation first
			case 'd':
				statusbar_backup = malloc(30 * sizeof(char));
				strcpy(statusbar_backup, statusbar);
				sprintf(statusbar, "%s", "Sure? y/n");
				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				char confirm_complete = getch();
				sprintf(statusbar, "%s", statusbar_backup);
				free(statusbar_backup);
				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				if(confirm_complete != 'y')
					break;

				solve_user_nums(sudoku_str,	user_nums);
				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				break;
			case 'e':
				if(small_mode)
					break;
				editing_notes = !editing_notes;
				char* mode = editing_notes == 1 ? "Note\0" : "Normal\0";
				sprintf(statusbar, "%s %s", mode, "Mode");

				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				break;
			//Exit; ask for confirmation
			case 'q':
				statusbar_backup = malloc(30 * sizeof(char));
				strcpy(statusbar_backup, statusbar);
				sprintf(statusbar, "%s", "Sure? y/n");
				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				char confirm_quit = getch();
				sprintf(statusbar, "%s", statusbar_backup);
				free(statusbar_backup);
				draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				if(confirm_quit != 'y')
					break;

				finish(0);
			//Input numbers into the user sudoku field
			default:
				//Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

				//Toggle the note fields
				if(editing_notes){
					if(key_press > 0x30 && key_press <= 0x39){
						int* target = &notes[((cursor.y * LINE_LEN * LINE_LEN) + (cursor.x * LINE_LEN)) + (key_press - 0x31)];
						*target = !*target;
						draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
					}
				//Check if the field is empty in the puzzle
				}else if(sudoku_str[cursor.y * LINE_LEN + cursor.x] == '0'){
					//check for numbers
					if(key_press >= 0x30 && key_press <= 0x39 && user_nums[cursor.y * LINE_LEN + cursor.x] != key_press){ 
						user_nums[cursor.y * LINE_LEN + cursor.x] = key_press;
						draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
				} //check for x
					else if(key_press == 'x' && user_nums[cursor.y * LINE_LEN + cursor.x] != '0'){
						user_nums[cursor.y * LINE_LEN + cursor.x] = '0';
						draw(statusbar, controls, notes, sudoku_str, user_nums, cursor);
					}
				}
				break;
		}
	}
}


int savestate(){
	FILE* savestate = fopen(filename, "w");

	if(savestate == NULL)
		return 0;

	fprintf(savestate, "%s\n%s\n", sudoku_str, user_nums);
	for(int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
		fprintf(savestate, "%d", notes[i]);
	fprintf(savestate, "\n");
	fclose(savestate);

	return 1;
}
