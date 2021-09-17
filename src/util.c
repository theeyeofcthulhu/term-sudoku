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

// curses init logic
void init_ncurses(){
    //on interrupt and segfault (Ctrl+c) exit (call finish)
	signal(SIGINT, finish);
	signal(SIGSEGV, finish);
    //init
	initscr();
    //return key doesn't become newline
	nonl();
    //allows Ctrl+c to quit the program
	cbreak();
    //don't echo the the getch() chars onto the screen
	noecho();
    //enable keypad (for arrow keys)
	keypad(stdscr, true);
    //color support
	if(!has_colors())
		finish_with_err_msg("Your terminal doesn't support color\n");
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
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
