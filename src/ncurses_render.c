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
char* statusbar;
char highlight = '0';
struct cursor cursor;

// curses init logic
void init_ncurses(){
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
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	init_pair(5, COLOR_BLACK, COLOR_BLUE);
}

// Draws everything
void draw(){
	erase();
	if(!render_small_mode)
		read_notes(notes);
	draw_sudokus(sudoku_str, user_nums);

	int string_y = render_small_mode ? LINE_LEN + 5 + PUZZLE_OFFSET : PUZZLE_OFFSET;
	int string_x = render_small_mode ? 0 : (LINE_LEN * 4) + 3 + PUZZLE_OFFSET;

	mvaddstr(string_y, string_x, statusbar);
	string_y += 2;

	// Split controls by '\n' characters to draw everything to the right of the puzzle when in big mode.
	// This has to be done because when drawing the whole string, only the first line is transposed to string_x.
	if(!render_small_mode){
		char* control_copy = malloc((strlen(controls) + 1) * sizeof(char));
		strcpy(control_copy, controls);

		char* control_line;
		// control_copy_ptr becomes NULL after first iteration since every call to strtok after the first one
		// has to be with NULL as str.
		for(char* control_copy_ptr = control_copy; (control_line = strtok(control_copy_ptr, "\n")) != NULL; control_copy_ptr = NULL, string_y++)
			mvaddstr(string_y, string_x, control_line);

		free(control_copy);
	}else
		mvaddstr(string_y, string_x, controls);


	move_cursor(cursor);
}


// For -v flag: show the generating process
void generate_visually(char* sudoku_to_display){
	erase();
	draw_border();
	attron(COLOR_PAIR(1));
	read_sudoku(sudoku_to_display, 1, 4);
	refresh();
	usleep(VISUAL_SLEEP);
}

// Draws the 'skeleton' of the sudoku:
// Number indicators on the sides, borders for large and small mode,
// different colors for indicating which is a block border
void draw_border(){
	if(!render_small_mode){
		for(int y = 0; y < LINE_LEN + 1; y++){
			// On every third vertical line, add colored seperator
			if(y % 3 == 0)
				attron(COLOR_PAIR(3));
			else
				attron(COLOR_PAIR(1));
			for(int i = 0; i < (LINE_LEN * 3) + LINE_LEN + 1; i++){
				mvaddch((y * 4) + PUZZLE_OFFSET, i + PUZZLE_OFFSET, '-');
			}
		}
		for(int x = 0; x < LINE_LEN + 1; x++){
			// On every third character, add a pipe
			if(x % 3 == 0)
				attron(COLOR_PAIR(3));
			else
				attron(COLOR_PAIR(1));
			for(int i = 1; i < LINE_LEN * 3 + LINE_LEN; i++){
				mvaddch(i + PUZZLE_OFFSET, (x * 4) + PUZZLE_OFFSET, '|');
			}
		}
		// Add horizontal seperators that overlay the others for indicating cube borders
		attron(COLOR_PAIR(3));
		for(int i = 0; i < (LINE_LEN * 3) + 10; i++){
			mvaddch((LINE_LEN * 3) + (3 * 3) + PUZZLE_OFFSET, i + PUZZLE_OFFSET, '-');
			mvaddch((LINE_LEN * 2) + (3 * 2) + PUZZLE_OFFSET, i + PUZZLE_OFFSET, '-');
			mvaddch((LINE_LEN * 1) + (3 * 1) + PUZZLE_OFFSET, i + PUZZLE_OFFSET, '-');
		}
		// Draw number indicators on the side
		int local_off = 0;
		for(int i = 0; i < LINE_LEN; i++){
			if(i % 3 == 0)
				local_off++;
			char* to_string = malloc(2 * sizeof(char));
			sprintf(to_string, "%d", i + 1);
			mvaddstr(i * 4 + PUZZLE_OFFSET + 2, 0, to_string);
			mvaddstr(0, i * 4 + PUZZLE_OFFSET + 2, to_string);
			free(to_string);
		}
	}else{
		// Draw borders for small mode
		for(int x = 0; x < 4; x++){
			for(int i = 0; i < LINE_LEN + 4; i++)
				mvaddch(i + PUZZLE_OFFSET, x * 4 + PUZZLE_OFFSET, '|');
		}
		for(int y = 0; y < 4; y++){
			for(int i = 0; i < LINE_LEN + 4; i++)
				mvaddch(y * 4 + PUZZLE_OFFSET, i + PUZZLE_OFFSET, '-');
		}
		// Draw number indicators on the side
		int local_off = 0;
		attron(COLOR_PAIR(3));
		for(int i = 0; i < LINE_LEN; i++){
			if(i % 3 == 0)
				local_off++;
			char* to_string = malloc(sizeof(char));
			sprintf(to_string, "%d", i + 1);
			mvaddstr(i + PUZZLE_OFFSET + local_off, 0, to_string);
			mvaddstr(0, i + PUZZLE_OFFSET + local_off, to_string);
			free(to_string);
		}
	}
}

// Read Sudoku to screen, respecting borders, etc.
void read_sudoku(char* sudoku, int color_mode, int color_mode_highlight){
	attron(COLOR_PAIR(color_mode));
	if(!render_small_mode){
		// Offset for counting in seperators when drawing numbers
		int yoff = PUZZLE_OFFSET;
		for(int y = 0; y < LINE_LEN; y++){
			// Increment offset
			yoff++;
			// Offset for horizontal seperators
			int xoff = PUZZLE_OFFSET;
			for(int x = 0; x < LINE_LEN; x++){
				xoff++;
				// Move into block
				move((y * 3) + yoff + 1, (x * 3) + xoff + 1);
				// Get digit from input string
				char current_digit = sudoku[y * LINE_LEN + x];
				// Draw everything except zeros
				if(current_digit != '0'){
					if(current_digit == highlight){
						attron(COLOR_PAIR(color_mode_highlight));
						addch(current_digit);
						attron(COLOR_PAIR(color_mode));
					}else
						addch(current_digit);
				}
			}
		}
	}else{
		// Offset for horizontal seperators
		int yoff = PUZZLE_OFFSET;
		for(int y = 0; y < LINE_LEN; y++){
			// On every third vertical line, add seperator
			if(y % 3 == 0){
				// Increment offset
				yoff++;
			}
			// Offset for horizontal seperators
			int xoff = PUZZLE_OFFSET;
			for(int x = 0; x < LINE_LEN; x++){
				if(x % 3 == 0){
					xoff++;
				}
				// Move into number position
				move(y + yoff, x + xoff);
				// Get digit from input string
				char current_digit = sudoku[y * LINE_LEN + x];
				// Draw everything except zeros
				if(current_digit != '0'){
					if(current_digit == highlight){
						attron(COLOR_PAIR(color_mode_highlight));
						addch(current_digit);
						attron(COLOR_PAIR(color_mode));
					}else
						addch(current_digit);
				}
			}
		}
	}
}

// Read the notes (nine switches for every cell) onto the screen
void read_notes(){
	attron(COLOR_PAIR(3));
	for(int i = 0; i < SUDOKU_LEN; i++){
		for(int j = 0; j < LINE_LEN; j++){
			if(notes[i * LINE_LEN + j])
				// Move into position for the note                                                                        Number as a character
				mvaddch(((i / LINE_LEN) * 4) + 1 + PUZZLE_OFFSET + (j / (LINE_LEN / 3)), ((i % LINE_LEN) * 4) + 1 + PUZZLE_OFFSET + (j % (LINE_LEN / 3)), j + 0x31);
		}
	}
}

// Move cursor but don't get into the seperators
void move_cursor_to(int x, int y){
	cursor.x = x;
	cursor.y = y;
	if(render_small_mode)
		move(cursor.y + (cursor.y / 3) + 1 + PUZZLE_OFFSET, cursor.x + (cursor.x / 3) + 1 + PUZZLE_OFFSET);
	else
		move((cursor.y * 4) + 2 + PUZZLE_OFFSET, (cursor.x * 4) + 2 + PUZZLE_OFFSET);
}

// Move cursor but don't get into the seperators
void move_cursor(){
	if(render_small_mode)
		move(cursor.y + (cursor.y / 3) + 1 + PUZZLE_OFFSET, cursor.x + (cursor.x / 3) + 1 + PUZZLE_OFFSET);
	else
		move((cursor.y * 4) + 2 + PUZZLE_OFFSET, (cursor.x * 4) + 2 + PUZZLE_OFFSET);
}

// Draw user numbers and the given sudoku in different colors
// Draw the user numbers under the given sudoku so the latter can't be overwritten
void draw_sudokus(){
	draw_border();

	read_sudoku(user_nums, 2, 5);

	read_sudoku(sudoku_str, 1, 4);
}
