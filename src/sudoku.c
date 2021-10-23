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

#include "sudoku.h"

#include "ncurses_render.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <stdbool.h>

typedef struct{
	char* vert_line;
	char* hor_line;
	char* block;
}sudoku_cell_props;

bool sudoku_gen_visual = false;
int sudoku_attempts = ATTEMPTS_DEFAULT;

char* user_nums;
char* sudoku_str;
int notes[SUDOKU_LEN * LINE_LEN + 1];

void solve_count(char* sudoku_to_solve, int* count);
void remove_nums(char* gen_sudoku);
bool solve(char* sudoku_str);
sudoku_cell_props get_cell_props(int cell, char* sudoku_str);
void free_cell_props(sudoku_cell_props cell);

// Given a cell in a sudoku get the according row, column and block
// Don't forget to free
sudoku_cell_props get_cell_props(int cell, char* sudoku_str){
	sudoku_cell_props result;

	result.vert_line = malloc(LINE_LEN * sizeof(char));
	memcpy(result.vert_line, sudoku_str + (LINE_LEN * (cell / LINE_LEN)), LINE_LEN * sizeof(char));

	result.hor_line = malloc(LINE_LEN * sizeof(char));
	for(int y = 0; y < LINE_LEN; y++)
		result.hor_line[y] = sudoku_str[y * LINE_LEN + (cell % LINE_LEN)];

	result.block = malloc(LINE_LEN * sizeof(char));
	for(int j = 0; j < LINE_LEN; j++)
		result.block[j] = sudoku_str[(((cell / LINE_LEN) / 3) * 3) * LINE_LEN + (((cell % LINE_LEN) / 3) * 3) + (LINE_LEN * (j / 3)) + (j % 3)];

	return result;
}

void free_cell_props(sudoku_cell_props cell){
	free(cell.vert_line);
	free(cell.hor_line);
	free(cell.block);
}

void init_sudoku_strings(){
	// Array for sudoku
	sudoku_str = malloc((SUDOKU_LEN) * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; sudoku_str[i++] = '0');

	// Array for numbers the user enters
	user_nums = malloc((SUDOKU_LEN) * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; user_nums[i++] = '0');

	// Array for notetaking
	for(int i = 0; i < SUDOKU_LEN * LINE_LEN; notes[i++] = 0);
}

// Generate a random sudoku
// This function generates the diagonal blocks from left to right and then calls solve()
// and remove_nums() to first fill out and then remove some numbers to create a complete puzzle
void generate_sudoku(char* gen_sudoku){
	if(sudoku_gen_visual)
		curs_set(0);
	// Fill each diagonal block with the values 1-9
	/* [x][ ][ ]
	 * [ ][x][ ]
	 * [ ][ ][x] */
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < LINE_LEN; j++){
			bool duplicate = true;
			while(duplicate){	
				duplicate = false;
				// Math to get a pointer to the current cell in the according diagonal block
				char* current_digit = &gen_sudoku[(i * 3) * LINE_LEN + (i * 3) + (LINE_LEN * (j / 3)) + (j % 3)];
				// Assign a random value between 1 and 9
				*current_digit = '1' + rand() % 9;
				// Check all digits up to 'j' cells into the block for duplicates
				for(int k = 0; k < j; k++){
					if(*current_digit == gen_sudoku[(i * 3) * LINE_LEN + (i * 3) + (LINE_LEN * (k / 3)) + (k % 3)])
						duplicate = true;
				}
			}
			// Draw the process of filling out the sudoku visually on the screen if that option is set via '-v'
			if(sudoku_gen_visual){
				generate_visually(gen_sudoku);
			}
		}
	}

	// Solve the remaining blocks
	solve(gen_sudoku);
	// Remove numbers but maintain unique solution
	remove_nums(gen_sudoku);

	if(sudoku_gen_visual)
		curs_set(1);
}

// Try and remove numbers until the solution is not unique
void remove_nums(char* gen_sudoku){
	int local_attempts = sudoku_attempts;
	int length = strlen(gen_sudoku) + 1;
	// Run down the attempts defined with '-n'
	while(local_attempts > 0){
		// Get non-empty cell
		int cell;
		bool found_non_empty = false;
		while(!found_non_empty){
			cell = rand() % SUDOKU_LEN;
			if(gen_sudoku[cell] != '0')
				found_non_empty = true;
		}

		// Generate a copy of the sudoku and count how many solutions it has
		char* sudoku_cpy = malloc(length * sizeof(char));
		strcpy(sudoku_cpy, gen_sudoku);
		sudoku_cpy[cell] = '0';
		int count = 0;
		solve_count(sudoku_cpy, &count);
		free(sudoku_cpy);

		// If unique, apply to real sudoku
		if(count == 1)
			gen_sudoku[cell] = '0';
		// Else, burn an attempt
		else
			local_attempts--;
	}
}

// Solve the sudoku on screen (fill in user_nums)
bool solve_user_nums(){
	// Combine entered numbers and puzzle to check if it is correct
	char* combined_solution = malloc((SUDOKU_LEN) * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; i++)
		combined_solution[i] = sudoku_str[i] == '0' ? user_nums[i] : sudoku_str[i];

	// Return true if the sudoku is already valid
	if(check_validity(combined_solution)){
		free(combined_solution);
		return true;
	}

	for(int i = 0; i < SUDOKU_LEN; i++){
		if(combined_solution[i] == '0'){
			// Get the fields associated with the value at i
			sudoku_cell_props cell_props = get_cell_props(i, combined_solution);

			// Try to assign a value to the cell at i
			for(int j = '1'; j <= '9'; j++){
				bool used = false;
				for(int k = 0; k < LINE_LEN; k++){
					// Check if the value is valid
					if(cell_props.vert_line[k] == j || cell_props.hor_line[k] == j || cell_props.block[k] == j)
						used = true;
				}
				// Try to recursively solve
				if(!used){
					user_nums[i] = j;
					// Check the whole path
					if(solve_user_nums()){
						free_cell_props(cell_props);
						free(combined_solution);
						return true;
					}

					// Otherwise, go back to 0
					user_nums[i] = '0';

				}
			}

			free_cell_props(cell_props);
			free(combined_solution);
			break;
		}
	}
	// Return false if no solution is found
	return false;
}

// Solve a sudoku (used in generating)
bool solve(char* sudoku_to_solve){
	// Draw the process of filling out the sudoku visually on the screen if that option is set via '-v'
	if(sudoku_gen_visual)
		generate_visually(sudoku_to_solve);

	// If sudoku is valid, return
	if(check_validity(sudoku_to_solve))
		return true;

	for(int i = 0; i < SUDOKU_LEN; i++){
		if(sudoku_to_solve[i] == '0'){
			// Get the fields associated with the value at start
			sudoku_cell_props cell_props = get_cell_props(i, sudoku_to_solve);

			// Try to assign a value to the cell at i
			for(int j = '1'; j <= '9'; j++){
				bool used = false;
				for(int k = 0; k < LINE_LEN; k++){
					// Check if the value is valid
					if(cell_props.vert_line[k] == j || cell_props.hor_line[k] == j || cell_props.block[k] == j)
						used = true;
				}
				if(!used){
					sudoku_to_solve[i] = j;
					// Check the whole path
					if(solve(sudoku_to_solve)){
						free_cell_props(cell_props);
						return true;
					}

					// Otherwise, go back to 0
					sudoku_to_solve[i] = '0';
				}
			}

			free_cell_props(cell_props);
			break;
		}
	}
	return false;
}

// Count the solutions to a puzzle
// Go through the puzzle recursively and increase count everytime you find a solution
void solve_count(char* sudoku_to_solve, int* count){

	// Function only needs to check if there is more than one unique
	// solution, so return if there is
	if(*count > 1)
		return;

	// Draw the process of filling out the sudoku visually on the screen if that option is set via '-v'
	if(sudoku_gen_visual)
		generate_visually(sudoku_to_solve);

	// find empty cell
	for(int i = 0; i < SUDOKU_LEN; i++){
		if(sudoku_to_solve[i] == '0'){
			// Get the fields associated with the value at i
			
			sudoku_cell_props cell_props = get_cell_props(i, sudoku_to_solve);

			// Try to assign a value to the cell at i
			for(int j = '1'; j <= '9'; j++){
				bool used = false;
				for(int k = 0; k < LINE_LEN; k++){
					// Check if the value is valid
					if(cell_props.vert_line[k] == j || cell_props.hor_line[k] == j || cell_props.block[k] == j)
						used = true;
				}
				if(!used){
					sudoku_to_solve [i] = j;
					// If assigning this value solved the grid, increase the count
					if(check_validity(sudoku_to_solve)){
						*count += 1;
						sudoku_to_solve[i] = '0';
						break;
					}
					// If not solved recursively call
					else{
						// Check the whole path
						solve_count(sudoku_to_solve, count);
					}

					// Otherwise, go back to 0
					sudoku_to_solve[i] = '0';
				}
			}

			free_cell_props(cell_props);
			break;
		}
	}

	return;
}

// Check for errors in the solved sudoku
bool check_validity(char* combined_solution){
	// Check for errors in vertical lines
	for(int i = 0; i < LINE_LEN; i++){
		char* vert_line = malloc(LINE_LEN * sizeof(char));
		// Copy the line from the combined string to vert_line
		memcpy(vert_line, combined_solution + (LINE_LEN * i), LINE_LEN * sizeof(char));

		// Check each digit against the others and check for zeros
		for(int j = 0; j < LINE_LEN; j++){
			char current_digit = vert_line[j];
			for(int k = 0; k < LINE_LEN; k++){
				if((vert_line[k] == current_digit && k != j) || vert_line[j] == '0'){
					free(vert_line);
					return false;
				}	
			}
		}
		free(vert_line);

		char* hor_line = malloc(LINE_LEN * sizeof(char));

		for(int y = 0; y < LINE_LEN; y++)
			hor_line[y] = combined_solution[y * LINE_LEN + i];

		// Check each digit against the others
		for(int i = 0; i < LINE_LEN; i++){
			char current_digit = hor_line[i];
			for(int j = 0; j < LINE_LEN; j++){
				if(hor_line[j] == current_digit && j != i){
					free(hor_line);
					return false;
				}
			}
		}
		free(hor_line);
	}

	// Check for errors in blocks
	for(int x = 0; x < LINE_LEN / 3; x++){
		for(int y = 0; y < LINE_LEN / 3; y++){
			char* block = malloc(LINE_LEN * sizeof(char));
			for(int i = 0; i < LINE_LEN; i++)
				block[i] = combined_solution[(y * 3) * LINE_LEN + (x * 3) + (LINE_LEN * (i / 3)) + (i % 3)];

			// Check each digit against the others
			for(int i = 0; i < LINE_LEN; i++){
				char current_digit = block[i];
				for(int j = 0; j < LINE_LEN; j++){
					if(block[j] == current_digit && j != i){
						free(block);
						return false;
					}	
				}
			}

			free(block);
		}
	}

	return true;
}
