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

// Exit ncurses cleanly

void finish(int sig){
	endwin();
	if(sig == SIGSEGV){
		printf("Segfault\n");
		exit(1);
	}
	exit(0);
}

void finish_with_err_msg(char* msg){
	endwin();
	printf("%s", msg);
	exit(1);
}

// List files in a directory into items (iterator will be returned as the actual size of items)
void listfiles(char* target_dir, char* items[STR_LEN], int* iterator){

	// Set iterator
	*iterator = 0;

	// Load contents of directory into items
	DIR *diretory_object;
	struct dirent *dir;
	diretory_object = opendir(target_dir);
	if(diretory_object){
		while((dir = readdir(diretory_object)) != NULL){
			if(!(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)){
				char* new_item = malloc(STR_LEN * sizeof(char));
				strcpy(new_item, dir->d_name);
				items[*iterator] = new_item;
				*iterator += 1;
			}
		}
		closedir(diretory_object);
	}else
		finish_with_err_msg("Couldn't open directory\n");

	if(*iterator == 0)
		finish_with_err_msg("No files available\n");
}
