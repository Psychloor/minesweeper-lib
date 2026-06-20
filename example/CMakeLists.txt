cmake_minimum_required(VERSION 3.16)

project(minesweeper-example
        VERSION 1.0.0
        LANGUAGES C)

add_executable(minesweeper-example main.c)

find_package(raylib CONFIG REQUIRED)

target_link_libraries(minesweeper-example PRIVATE raylib minesweeper-lib::minesweeper-lib)

if (WIN32)
    target_link_libraries(minesweeper-example PRIVATE winmm)
endif ()