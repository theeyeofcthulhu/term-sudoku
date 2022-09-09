cmake_minimum_required(VERSION 3.10)

project(term-sudoku C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "-ggdb -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -Wshadow -pedantic")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
  "${SRC_DIR}/main.c"
  "${SRC_DIR}/ncurses_render.c"
  "${SRC_DIR}/sudoku.c"
  "${SRC_DIR}/util.c"
  )

add_executable(term-sudoku ${SOURCES})
target_link_libraries(term-sudoku ncurses)

install(TARGETS term-sudoku)
