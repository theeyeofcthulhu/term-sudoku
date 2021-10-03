CC = gcc
LIBS = -lncurses

SRC = src/*.c
INCLUDE = src/*.h
EXE = $(ODIR)/term-sudoku
ODIR = bin

PREFIX = /usr/local
MANPATH = $(PREFIX)/share/man

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

$(EXE): $(SRC) $(INCLUDE) | $(ODIR)
	$(CC) -o $@ $(SRC) $(LIBS)
