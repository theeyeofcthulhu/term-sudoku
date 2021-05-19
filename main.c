#include "main.h"

struct cursor{
	int y;
	int x;
};

char* filename;
char* user_nums;
char* sudoku_str;
struct cursor cursor;
char* statusbar;
char* controls;
char* sudoku;

int main(int argc, char **argv){
	time_t t;
	srand((unsigned) time(&t));

	generate_sudoku();

	//--- USER INIT LOGIC ---
	
	filename = argv[1];

	//Read Sudoku from given file
	FILE* input_file = fopen(argv[1], "r");
	if(input_file == NULL)
		finish_with_err_msg("Error accessing file\n");

	sudoku_str = malloc((SUDOKU_LEN) * sizeof(char));
	user_nums = malloc((SUDOKU_LEN) * sizeof(char));

	fscanf(input_file, "%s\n%s", sudoku_str, user_nums);
	fclose(input_file);

	//Abort if Sudoku is wrong length
	if((strlen(sudoku_str) + strlen(user_nums)) != (SUDOKU_LEN * 2))
		finish_with_err_msg("Wrong length in Sudoku file\n");

	sudoku_str = sudoku;

	cursor.x = 0;
	cursor.y = 0;

	statusbar = malloc(30 * sizeof(char));
	sprintf(statusbar, "%s", "File opened");

	controls =	"move - h, j, k and l\n"
				"1-9 - insert numbers\n"
				"x or 0 - delete numbers\n"
				"save - s\n"
				"check for errors - c\n"
				"quit - q";

	//--- CURSES INIT LOGIC ---

    //on interrupt and segfault (Ctrl+c) exit (call finish)
	signal(SIGINT, finish);
	signal(SIGSEGV, finish);
    //init
	initscr();
    //return key doesn't become newline
	nonl();
    //cursor
	curs_set(1);
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

	draw();

	//Main loop: wait for keypress, then process it
	while(true){
		char key_press = getch();

		char* combined_solution;

		//Move on vim keys and bind to field size
		switch(key_press){
			case 'h':
				cursor.x = cursor.x - 1 < 0 ? cursor.x : cursor.x - 1;
				break;
			case 'j':
				cursor.y = cursor.y + 1 >= LINE_LEN ? cursor.y : cursor.y + 1 ;
				break;
			case 'k':
				cursor.y = cursor.y - 1 < 0 ? cursor.y : cursor.y - 1;
				break;
			case 'l':
				cursor.x = cursor.x + 1 >= LINE_LEN ? cursor.x : cursor.x + 1 ;
				break;
			case 's':
				//Save file and handle errors
				if(!savestate())
					finish_with_err_msg("Error accessing file\n");
				else
					sprintf(statusbar, "%s", "Saved");
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
				break;

				free(combined_solution);
			case 'q':
				finish(0);
			//Input numbers into the user sudoku field
			default:
				//Check if the key is a number (not zero) in aasci chars or 'x' and if the cursor is not an a field filled by the puzzle

				//Check if the field is empty in the puzzle
				if(sudoku_str[cursor.y * LINE_LEN + cursor.x] == '0'){
					//check for numbers
					if(key_press >= 0x30 && key_press <= 0x39 && user_nums[cursor.y * LINE_LEN + cursor.x] != key_press){ 
						user_nums[cursor.y * LINE_LEN + cursor.x] = key_press;
						sprintf(statusbar, "%s", "*Not saved");
					}
					//check for x
					else if(key_press == 'x' && user_nums[cursor.y * LINE_LEN + cursor.x] != '0'){
						user_nums[cursor.y * LINE_LEN + cursor.x] = '0';
						sprintf(statusbar, "%s", "*Not saved");
					}
				}
				break;
		}

		draw();

		move_cursor();
	}
}

//Generate a random sudoku
//This function generates the diagonal blocks from left to right and then calls fill_remaining()
void generate_sudoku(){
	sudoku = malloc(SUDOKU_LEN * sizeof(char));
	for(int i = 0; i < SUDOKU_LEN; i++)
		sudoku[i] = '0';

	//Fill each diagonal block with the values 1-9
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < LINE_LEN; j++){
			int duplicate = 1;
			while(duplicate){	
				duplicate = 0;
				char* current_digit = &sudoku[(i * 3) * LINE_LEN + (i * 3) + (LINE_LEN * (j / 3)) + (j % 3)];
				*current_digit = 0x31 + rand() % 9;
				for(int k = 0; k < j; k++){
					if(*current_digit == sudoku[(i * 3) * LINE_LEN + (i * 3) + (LINE_LEN * (k / 3)) + (k % 3)])
						duplicate = 1;
				}
			}
		
		}
	}

	fill_remaining(0);
	remove_nums();
}

//Fill the cells recursively
int fill_remaining(int start){
	//Check if we are finished
	if(start >= SUDOKU_LEN)
		return true;

	//If start is in a diagonal block, go to next i
	if(((start % LINE_LEN) / 3) == ((start / LINE_LEN) / 3))
		return fill_remaining(start + 1);

	//Get the fields associated with the value at start
	
	char* vert_line = malloc(LINE_LEN * sizeof(char));
	memcpy(vert_line, sudoku + (LINE_LEN * (start / LINE_LEN)), LINE_LEN * sizeof(char));

	char* hor_line = malloc(LINE_LEN * sizeof(char));
	for(int y = 0; y < LINE_LEN; y++){
		hor_line[y] = sudoku [y * LINE_LEN + (start % LINE_LEN)];
	}

	char* block = malloc(LINE_LEN * sizeof(char));
	for(int j = 0; j < LINE_LEN; j++){
		block[j] = sudoku [(((start / LINE_LEN) / 3) * 3) * LINE_LEN + (((start % LINE_LEN) / 3) * 3) + (LINE_LEN * (j / 3)) + (j % 3)];
	}

	//Try to assign a value to the cell at start
	for(int j = 0x31; j <= 0x39; j++){
		int used = 0;
		for(int k = 0; k < LINE_LEN; k++){
			//Check if the value is valid
			if(vert_line[k] == j || hor_line[k] == j || block[k] == j)
				used = 1;
		}
		if(!used){
			sudoku[start] = j;
			//Check the whole path
			if(fill_remaining(start + 1)){
				free(vert_line);
				free(hor_line);
				free(block);
				return true;
			}

			//Otherwise, go back to 0
			sudoku[start] = '0';
		}
	}

	free(vert_line);
	free(hor_line);
	free(block);
	return false;
}

void remove_nums(){
	for(int i = 0; i < 40; i++){
		int number_to_rm = rand() % SUDOKU_LEN;
		if(sudoku[number_to_rm] != '0'){
			sudoku[number_to_rm] = '0';
		}
	}
}

void draw(){
	erase();
	draw_sudokus(user_nums, sudoku_str);
	mvaddstr(LINE_LEN + 4, 0, statusbar);
	mvaddstr(LINE_LEN + 6, 0, controls);
	move_cursor(cursor);
}

//Write changed values back to file
int savestate(){
	FILE* savestate = fopen(filename, "w");

	if(savestate == NULL)
		return 0;

	fprintf(savestate, "%s\n%s", sudoku_str, user_nums);
	fclose(savestate);

	return 1;
}

//Check for errors in the sudoku and write to statusbar
int check_validity(char* combined_solution){
	//Combine user_nums and sudoku_str by replacing the 0s in sudoku_str with the user_nums
	for(int i = 0; i < SUDOKU_LEN; i ++){
		if(combined_solution[i] == '0'){
			return 0;
		}
	}
	
	//Check for errors in vertical lines
	for(int i = 0; i < LINE_LEN; i++){
		char* vert_line = malloc(LINE_LEN * sizeof(char));
		//Copy the line from the combined string to vert_line
		memcpy(vert_line, combined_solution + (LINE_LEN * i), LINE_LEN * sizeof(char));

		//Check each digit against the others
		for(int j = 0; j < LINE_LEN; j++){
			for(int k = 0; k < LINE_LEN; k++){
				if(vert_line[k] == vert_line[j] && k != j){
					free(vert_line);
					return 0;
				}	
			}
		}
		free(vert_line);
	}
	
	//Check for errors in horizontal lines
	for(int x = 0; x < LINE_LEN; x++){
		char* hor_line = malloc(LINE_LEN * sizeof(char));

		for(int y = 0; y < LINE_LEN; y++){
			hor_line[y] = combined_solution[y * LINE_LEN + x];
		}

		//Check each digit against the others
		for(int i = 0; i < LINE_LEN; i++){
			char current_digit = hor_line[i];
			for(int j = 0; j < LINE_LEN; j++){
				if(hor_line[j] == hor_line[i] && j != i){
					free(hor_line);
					return 0;
				}	
			}
		}
		free(hor_line);
	}

	//Check for errors in blocks
	for(int x = 0; x < LINE_LEN / 3; x++){
		for(int y = 0; y < LINE_LEN / 3; y++){
			char* block = malloc(LINE_LEN * sizeof(char));
			for(int i = 0; i < LINE_LEN; i++){
				block[i] = combined_solution[(y * 3) * LINE_LEN + (x * 3) + (LINE_LEN * (i / 3)) + (i % 3)];
			}

			//Check each digit against the others
			for(int i = 0; i < LINE_LEN; i++){
				char current_digit = block[i];
				for(int j = 0; j < LINE_LEN; j++){
					if(block[j] == block[i] && j != i){
						free(block);
						return 0;
					}	
				}
			}

			free(block);
		}
	}

	return 1;
}

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

//Read Sudoku to screen
void read_sudoku(char* sudoku){
	//Offset for horizontal seperators
	int yoff = 0;
	for(int y = 0; y < LINE_LEN; y++){
		//On every third vertical line, add seperator
		if(y % 3 == 0){
			//Add 4 horizontal seperators 
			for(int i = 0; i < LINE_LEN + 4; i++)
				mvaddch(y + yoff, i, '-');
			//Increment offset
			yoff++;
		}
		//Offset for vertical seperators
		int xoff = 0;
		for(int x = 0; x < LINE_LEN; x++){
			//On every third character, add a pipe
			if(x % 3 == 0){
				//Move, print and increment
				mvaddch(y + yoff, x + xoff++, '|');
			}
			move(y + yoff, x + xoff);
			//Get digit from input string
			char current_digit = sudoku[y * LINE_LEN + x];
			//Draw everything except zeros
			if(current_digit != '0')
				addch(current_digit);
		}
		//Add vertical seperator add the end
		mvaddch(y + yoff, LINE_LEN + 3, '|');
	}
	//Add horizontal seperator add the end
	for(int i = 0; i < LINE_LEN + 4; i++){
		mvaddch(LINE_LEN + yoff, i, '-');
	}
}

//Move cursor but don't get into the seperators
void move_cursor(){
	move(cursor.y + (cursor.y / 3) + 1, cursor.x + (cursor.x / 3) + 1);
}

//Draw user numbers and the given sudoku in different colors
//Draw the user numbers under the given sudoku so the latter can't be overwritten
void draw_sudokus(){
	attron(COLOR_PAIR(2));
	read_sudoku(user_nums);

	attron(COLOR_PAIR(1));
	read_sudoku(sudoku_str);
}
