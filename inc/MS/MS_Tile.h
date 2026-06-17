//
// Created by blomq on 2026-06-16.
//

#ifndef MINESWEEPER_LIB_TILE_H
#define MINESWEEPER_LIB_TILE_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MS_Tile {
    bool isOpen: 1;
    bool isMine: 1;
    bool isFlagged: 1;
    bool isQuestionMarked: 1;
    uint8_t adjacentMines: 4;
} MS_Tile;

#ifdef __cplusplus
}
#endif

#endif //MINESWEEPER_LIB_TILE_H
