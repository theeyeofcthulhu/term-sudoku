CC = gcc
LIBS = -lncurses

SRC = $(addprefix src/,main.c util.c sudoku.c ncurses_render.c)
INCLUDE = $(addprefix src/,main.h util.h sudoku.h ncurses_render.h)
OBJ = $(addprefix bin/,$(notdir src/$(SRC:.c=.o)))
EXE = $(ODIR)/term-sudoku
ODIR = bin
SRCDIR = src

DESTDIR = /usr
MANPATH = $(DESTDIR)/share/man

build: $(EXE)

run: $(EXE)
	bin/term-sudoku

file: $(EXE)
	bin/term-sudoku -f

own: $(EXE)
	bin/term-sudoku -e

small: $(EXE)
	bin/term-sudoku -sc

install: $(EXE)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(ODIR)/term-sudoku $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPATH)/man1
	cp -f term-sudoku.1 $(DESTDIR)$(MANPATH)/man1/term-sudoku.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/term-sudoku
	rm -f $(MANPATH)/man1/term-sudoku.1

clean:
	rm -rf $(ODIR)

$(ODIR):
	mkdir -p $@

$(EXE): $(OBJ) $(INCLUDE) | $(ODIR)
	$(CC) -o $@ $(OBJ) $(LIBS)

$(ODIR)/%.o: $(SRCDIR)/%.c* $(SRCDIR)/%.h* | $(ODIR)
	$(CC) -o $@ -c $<
