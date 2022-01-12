# Term-Sudoku

This is a terminal application to play the puzzle game of Sudoku.

## Build

### Prerequisites

- [ncurses](https://invisible-island.net/ncurses/ncurses.html)

### Build process

Install CMake and Make.

`$ ./build.sh`

Creates a "build" folder with the binary.

`$ make -C build install`

copies that binary to /usr/local/bin.

## Arch User Repository (AUR)

Install via AUR (substitute paru with an AUR wrapper of your choice)

`$ paru -Syu term-sudoku`

## Usage

When you start the program with

`$ build/term-sudoku`

a sudoku is generated for you (run with '-v' flag to see that process).  Using
the controls displayed on the screen insert numbers and move around.

When you save by pressing 's' the Sudoku is saved to ~/.local/share/term-sudoku/
or any directory specified with the '-d' flag.

Continue where you left of by starting with the -f flag

`$ build/term-sudoku -f`

and choosing your file.

See more flags and help with the -h flag.

You start in "normal mode". Meaning you type in numbers for the solution into
the grid. When pressing 'e' you get into "Note Mode", where you can note numbers
that would fit into that square. Every number there is like a switch: press a
number to toggle it's visibility as a note.

## Generation of the Sudoku

To see this process run with the '-v' flag. What happens is from left to right and top
to bottom the first three blocks are filled with the values 1 to 9 randomly.
This pattern follows the sudoku rules as no vertical or horizontal lines
interfere. Then, using backtracking the rest of the squares are filled, leaving
us with a fully filled sudoku grid.  Afterwards numbers are removed one by one
and, using backtracking, the solutions to the puzzle with the removed numbers
are counted.  Once there is more than one solution the removal is stopped.
