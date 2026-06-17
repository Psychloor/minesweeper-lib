cmake_minimum_required(VERSION 3.16)

project(minesweeper_lib
        VERSION 1.0.0
        LANGUAGES C)

add_library(minesweeper_lib STATIC
        src/MS_Minefield.c
        inc/MS/Minesweeper.h
        inc/MS/MS_point.h
        inc/MS/MS_GameState.h
        inc/MS/MS_TileSprite.h)

add_library(minesweeper_lib::minesweeper_lib ALIAS minesweeper_lib)

target_include_directories(minesweeper_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>)

target_compile_features(minesweeper_lib
        PUBLIC
        c_std_99)