cmake_minimum_required(VERSION 3.16)

project(minesweeper-lib
        VERSION 1.0.0
        LANGUAGES C)

include(GenerateExportHeader)

# Support BUILD_SHARED_LIBS or explicit option
option(MINESWEEPER_BUILD_SHARED "Build as shared library" ${BUILD_SHARED_LIBS})
option(MINESWEEPER_BUILD_EXAMPLE "Build example (requires raylib)")

if(MINESWEEPER_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

add_library(minesweeper-lib ${LIB_TYPE}
        src/MS_Minefield.c
        inc/MS/Minesweeper.h
        inc/MS/MS_Point.h
        inc/MS/MS_GameState.h
        inc/MS/MS_TileSprite.h
        inc/MS/MS_Minefield.h
        inc/MS/MS_Tile.h)

generate_export_header(minesweeper-lib
        BASE_NAME MS
        EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/inc/MS/MS_export.h"
)

add_library(minesweeper-lib::minesweeper-lib ALIAS minesweeper-lib)

target_include_directories(minesweeper-lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>)

target_compile_features(minesweeper-lib
        PUBLIC
        c_std_99)

set(MINESWEEPER_BUILD_EXAMPLE ON)
if(MINESWEEPER_BUILD_EXAMPLE)
    add_subdirectory(example EXCLUDE_FROM_ALL)
endif ()