# Term-Sudoku

This is a UNIX-terminal application to play the game of Sudoku.

## Build

### Prerequisites

- [ncurses](https://invisible-island.net/ncurses/ncurses.html)

### Makefile

`$ make build`

creates a "bin" folder with the binary.

`$ make install`

copies that binary to /usr/local/bin.

## Arch User Repository (AUR)

Install via AUR (using paru)

`$ paru -Syu term-sudoku`

## Usage

When you start the program with

`$ make run` or `$ bin/term-sudoku`

a sudoku is generated for you (run with '-v' flag to see that process).
Using the controls displayed on the screen insert numbers.
It's going to look like this:

![Example](https://raw.githubusercontent.com/theeyeofcthulhu/term-sudoku/master/example.png)

When you save with s a file like this is generated (the first line is the
puzzle, the second is the solution the user fills in, the third is the notes (9 per cell)):

```
000004080002350400410800200004000803630000021058010074090001008000002006706038050
000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```

Continue where you left of by starting with the -f flag

`$ bin/term-sudoku -f`

and choosing your file.

See more flags and help with the -h flag.

You start in "normal mode". Meaning you type in numbers for the solution
into the grid. When pressing e you get into "Note Mode", where you can
note numbers that would fit into that square. Every number there is like
a switch: press a number to toggle it's visibility as a note.

## Generation of the Sudoku

To see this process run with -v flag. What happens is from left to right
and top to bottom the first three blocks are filled with the values 1 to 9
randomly.  This pattern follows the sudoku rules as no vertical or horizontal
lines interfere. Then, using backtracking the rest of the squares are filled,
leaving us with a fully filled sudoku grid.  Afterwards numbers are removed one
by one and, using backtracking, the solutions to the puzzle with the removed
numbers are counted.  Once there is more than one solution the removal is
stopped.
