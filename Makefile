CC = gcc
CCFLAGS = -g -Wall
LIBS = $(addprefix -l,ncurses)

SRC = $(addprefix src/,main.c util.c sudoku.c ncurses_render.c)
INCLUDE = $(addprefix src/,main.h util.h sudoku.h ncurses_render.h)
OBJ = $(addprefix bin/,$(notdir src/$(SRC:.c=.o)))
EXE = $(BINDIR)/term-sudoku
BINDIR = bin
SRCDIR = src

PREFIX = /usr/local
MANPATH = $(PREFIX)/share/man

.PHONY: all
all: $(EXE)

.PHONY: run
run: $(EXE)
	bin/term-sudoku

.PHONY: file
file: $(EXE)
	bin/term-sudoku -f

.PHONY: own
own: $(EXE)
	bin/term-sudoku -e

.PHONY: small
small: $(EXE)
	bin/term-sudoku -sc

.PHONY: install
install: $(EXE)
	mkdir -p $(PREFIX)/bin
	cp -f $(BINDIR)/term-sudoku $(PREFIX)/bin
	mkdir -p $(MANPATH)/man1
	cp -f term-sudoku.1 $(MANPATH)/man1/term-sudoku.1

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/term-sudoku
	rm -f $(MANPATH)/man1/term-sudoku.1

.PHONY: clean
clean:
	rm -rf $(BINDIR)

$(BINDIR):
	mkdir -p $@

$(BINDIR)/main.o: src/main.h src/util.h src/sudoku.h src/ncurses_render.h
$(BINDIR)/ncurses_render.o: src/main.h src/util.h src/sudoku.h src/ncurses_render.h
$(BINDIR)/util.o: src/util.h src/main.h src/util.h src/ncurses_render.h
$(BINDIR)/sudoku.o: src/main.h src/sudoku.h src/ncurses_render.h

$(EXE): $(OBJ) $(INCLUDE) | $(BINDIR)
	$(CC) $(CCFLAGS) -o $@ $(OBJ) $(LIBS)

$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) -c $(CCFLAGS) -o $@ $<
