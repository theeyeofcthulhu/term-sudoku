cmake_minimum_required(VERSION 3.10)

project(term-sudoku C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "-ggdb -Wall -Wextra -Wshadow -Wno-format-truncation -pedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -Wshadow -Wno-format-truncation -pedantic -DNDEBUG")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
  "${SRC_DIR}/main.c"
  "${SRC_DIR}/ncurses_render.c"
  "${SRC_DIR}/sudoku.c"
  "${SRC_DIR}/util.c"
  )

add_executable(term-sudoku ${SOURCES})
target_link_libraries(term-sudoku ncurses)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/term-sudoku.1 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/term-sudoku.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)

install(TARGETS term-sudoku)
