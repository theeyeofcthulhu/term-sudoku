#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>

#define STR_LEN 80

void finish(int sig);
void finish_with_err_msg(char* msg);
void listfiles(char* target_dir, char* items[STR_LEN], int* iterator);
void init_ncurses();
