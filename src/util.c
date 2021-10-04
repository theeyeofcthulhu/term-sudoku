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

#include "util.h"

bool util_ask_confirmation = true;

// Exit ncurses cleanly

void finish(int sig){
	endwin();
	if(sig == SIGSEGV){
		printf("Segfault\n");
		exit(1);
	}
	exit(0);
}

// Takes the same parameters as printf and exits ncurses as well as the application
void finish_with_err_msg(char* msg, ...){
	endwin();

	// Send '...'-args to vprintf
	va_list format;
	va_start(format, msg);
	vprintf(msg, format);
	va_end(format);

	exit(1);
}

// List files in a directory into items (iterator will be returned as the actual size of items)
void listfiles(char* target_dir, char* items[], int* iterator){

	// Set iterator
	*iterator = 0;

	// Load contents of directory into items
	DIR *diretory_object;
	struct dirent *dir;
	diretory_object = opendir(target_dir);
	if(diretory_object){
		while((dir = readdir(diretory_object)) != NULL){
			if(!(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) && strlen(dir->d_name) < STR_LEN){
				char* new_item = malloc(STR_LEN * sizeof(char));
				strcpy(new_item, dir->d_name);
				items[*iterator] = new_item;
				*iterator += 1;
			}
		}
		closedir(diretory_object);
	}else
		finish_with_err_msg("Directory '%s' could not be opened or does not exist\n", target_dir);
}

// Write sudoku_str, user_nums and notes to file
bool savestate(char* filename){
	FILE* savestate = fopen(filename, "w");

	if(savestate == NULL)
		return false;

	fprintf(savestate, "%s\n%s\n", sudoku_str, user_nums);
	for(int i = 0; i < SUDOKU_LEN * LINE_LEN; i++)
		fprintf(savestate, "%d", notes[i]);
	fprintf(savestate, "\n");
	fclose(savestate);

	return true;
}

// Ask for confirmation by displaying
bool status_bar_confirmation(){
	// Return if the '-c' flag is set (the user does not want to be asked)
	if(!util_ask_confirmation)
		return true;

	char* statusbar_backup = malloc((strlen(statusbar) + 1) * sizeof(char));
	strcpy(statusbar_backup, statusbar);

	sprintf(statusbar, "%s", "Sure? y/n");
	draw();

	char confirm = getch();

	sprintf(statusbar, "%s", statusbar_backup);
	free(statusbar_backup);
	draw();

	if(confirm != 'y')
		return false;

	return true;
}
