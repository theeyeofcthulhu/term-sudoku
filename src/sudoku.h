#include <string.h>
#include <stdlib.h>

#define LINE_LEN 9
#define SUDOKU_LEN 81
#define ATTEMPTS_DEFAULT 5
#define VISUAL_SLEEP 10000
#define STR_LEN 80

extern int sudoku_gen_visual;
extern int sudoku_attempts;

struct sudoku_cell_props;

//Sudoku generating and other sudoku-related things
struct sudoku_cell_props get_cell_props(int cell, char* sudoku_str);
void generate_sudoku(char* gen_sudoku);
int fill_remaining(int start);
void remove_nums(char* gen_sudoku);
int solve(char* sudoku_str);
void solve_count(char* sudoku_to_solve, int* count);
int solve_user_nums();
void generate_visually(char* sudoku_to_display);
int check_validity(char* combined_solution);
