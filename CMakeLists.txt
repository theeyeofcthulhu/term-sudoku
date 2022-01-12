cmake_minimum_required(VERSION 3.10)

project(term-sudoku C)

set(CMAKE_C_FLAGS "-g -Wall -Wextra -Wshadow -Wno-format-truncation -std=c11 -pedantic -D_POSIX_C_SOURCE=200809")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "${SRC_DIR}/*.c")

add_executable(term-sudoku ${SOURCES})
target_link_libraries(term-sudoku ncurses)

install(TARGETS term-sudoku)
