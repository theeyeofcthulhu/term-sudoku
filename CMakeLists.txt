cmake_minimum_required(VERSION 3.10)

project(term-sudoku C)

set(CMAKE_C_FLAGS_DEBUG "-ggdb -Wall -Wextra -Wshadow -std=c11 -pedantic -D_POSIX_C_SOURCE=200809")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -Wshadow -std=c11 -pedantic -D_POSIX_C_SOURCE=200809")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "${SRC_DIR}/*.c")

add_executable(term-sudoku ${SOURCES})
target_link_libraries(term-sudoku ncurses)

install(TARGETS term-sudoku)
