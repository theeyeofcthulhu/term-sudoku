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
along with this program.  If not, see <https:// www.gnu.org/licenses/>.
*/

#include "ncurses_render.h"

bool render_small_mode = false;

// curses init logic
void init_ncurses(){
    // on interrupt and segfault (Ctrl+c) exit (call finish)
	signal(SIGINT, finish);
	signal(SIGSEGV, finish);
    // init
	initscr();
    // return key doesn't become newline
	nonl();
    // allows Ctrl+c to quit the program
	cbreak();
    // don't echo the the getch() chars onto the screen
	noecho();
    // enable keypad (for arrow keys)
	keypad(stdscr, true);
    // color support
	if(!has_colors())
		finish_with_err_msg("Your terminal does not support color\n");
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}

void draw(char* statusbar, char* controls, int* notes, char* sudoku_str, char* user_nums, struct cursor cursor){
	erase();
	if(!render_small_mode)
		read_notes(notes);
	draw_sudokus(sudoku_str, user_nums);
	int string_off_set = render_small_mode ? LINE_LEN + 5 : (LINE_LEN + 3) * 3 + 2;
	mvaddstr(string_off_set, 0, statusbar);
	mvaddstr(string_off_set + 2, 0, controls);
	move_cursor(cursor);
}


// For -v flag: show the generating process
void generate_visually(char* sudoku_to_display){
	erase();
	read_sudoku(sudoku_to_display, 1);
	refresh();
	usleep(VISUAL_SLEEP);
}

// Write changed values back to file
// Read Sudoku to screen, adding seperators between the blocks for visuals
void read_sudoku(char* sudoku, int color_mode){
	if(!render_small_mode){
		// Offset for counting in seperators when drawing numbers
		int yoff = 0;
		for(int y = 0; y < LINE_LEN; y++){
		// On every third vertical line, add seperator
			// Add vertical seperators
			//                 normal length    Account for vertical separators
			for(int i = 0; i < (LINE_LEN * 3) + LINE_LEN + 1; i++){
				if(y % 3 == 0)
					attron(COLOR_PAIR(3));
				mvaddch((y * 4), i, '-');
				attron(COLOR_PAIR(color_mode));
			}
			// Increment offset
			yoff++;
			// Offset for horizontal seperators
			int xoff = 0;
			for(int x = 0; x < LINE_LEN; x++){
				// On every third character, add a pipe
				// Move, print and increment       Account for separators in between blocks
				for(int i = 1; i < LINE_LEN * 3 + LINE_LEN; i++){
					if(x % 3 == 0)
						attron(COLOR_PAIR(3));
					mvaddch(i, (x * 4), '|');
					attron(COLOR_PAIR(color_mode));
				}
				xoff++;
				// Move into block
				move((y * 3) + yoff + 1, (x * 3) + xoff + 1);
				// Get digit from input string
				char current_digit = sudoku[y * LINE_LEN + x];
				// Draw everything except zeros
				if(current_digit != '0')
					addch(current_digit);
			}
			// Add vertical seperator add the end
			for(int i = 1; i < LINE_LEN * 3 + 9; i++){
				attron(COLOR_PAIR(3));
				mvaddch(i, (LINE_LEN * 3) + 9, '|');
				attron(COLOR_PAIR(color_mode));
			}
		}
		// Add horizontal seperators that overlay for indicating cube borders
		attron(COLOR_PAIR(3));
		for(int i = 0; i < (LINE_LEN * 3) + 10; i++){
			mvaddch((LINE_LEN * 3) + (3 * 3), i, '-');
			mvaddch((LINE_LEN * 2) + (3 * 2), i, '-');
			mvaddch((LINE_LEN * 1) + (3 * 1), i, '-');
		}
		attron(COLOR_PAIR(color_mode));
	}else{
		// Offset for horizontal seperators
		int yoff = 0;
		for(int y = 0; y < LINE_LEN; y++){
			// On every third vertical line, add seperator
			if(y % 3 == 0){
				// Add 4 vertical seperators
				for(int i = 0; i < LINE_LEN + 4; i++)
					mvaddch(y + yoff, i, '-');
				// Increment offset
				yoff++;
			}
			// Offset for horizontal seperators
			int xoff = 0;
			for(int x = 0; x < LINE_LEN; x++){
				// On every third character, add a pipe
				if(x % 3 == 0){
					// Move, print and increment
					mvaddch(y + yoff, x + xoff++, '|');
				}
				// Move into number position
				move(y + yoff, x + xoff);
				// Get digit from input string
				char current_digit = sudoku[y * LINE_LEN + x];
				// Draw everything except zeros
				if(current_digit != '0')
					addch(current_digit);
			}
			// Add vertical seperator add the end
			mvaddch(y + yoff, LINE_LEN + 3, '|');
		}
		// Add horizontal seperator add the end
		for(int i = 0; i < LINE_LEN + 4; i++){
			mvaddch(LINE_LEN + yoff, i, '-');
		}
	}
}

void read_notes(int* notes){
	attron(COLOR_PAIR(3));
	for(int i = 0; i < SUDOKU_LEN; i++){
		for(int j = 0; j < LINE_LEN; j++){
			if(notes[i * LINE_LEN + j])
				// Move into position for the note                                                                        Number as a character
				mvaddch(((i / LINE_LEN) * 4) + 1 + (j / (LINE_LEN / 3)), ((i % LINE_LEN) * 4) + 1 + (j % (LINE_LEN / 3)), j + 0x31);
		}
	}
	attron(COLOR_PAIR(1));
}

// Move cursor but don't get into the seperators
void move_cursor(struct cursor cursor){
	if(render_small_mode)
		move(cursor.y + (cursor.y / 3) + 1, cursor.x + (cursor.x / 3) + 1);
	else
		move((cursor.y * 4) + 2, (cursor.x * 4) + 2);
}

// Draw user numbers and the given sudoku in different colors
// Draw the user numbers under the given sudoku so the latter can't be overwritten
void draw_sudokus(char* sudoku_str, char* user_nums){
	attron(COLOR_PAIR(2));
	read_sudoku(user_nums, 2);

	attron(COLOR_PAIR(1));
	read_sudoku(sudoku_str, 1);
}
