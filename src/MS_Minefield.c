#include "MS/MS_Minefield.h"

#include <assert.h>

#include "MS/MS_Set.h"
#include "MS/MS_Vector.h"

#include <stdlib.h>
#include <time.h>

MS_VECTOR_DEFINE(MS_Point, ms_234564334534_PointVector);

static uint32_t ms_234564334534_int_hash(const int value) {
    return (uint32_t) value * 2654435761u;
}

static bool ms_234564334534_int_equals(const int a, const int b) {
    return a == b;
}

MS_SET_DEFINE(int, ms_234564334534_IntSet, ms_234564334534_int_hash, ms_234564334534_int_equals);

bool ms_234564334534_isInsideRange(const MS_Minefield *minefield, const int xPos, const int yPos) {
    return xPos >= 0 && xPos < minefield->width && yPos >= 0 && yPos < minefield->height;
}

int ms_234564334534_clamp(const int value, const int min, const int max) {
    return value < min ? min : value > max ? max : value;
}

void ms_234564334534_placeMines(const MS_Minefield *minefield, const int firstX, const int firstY) {
    // for a minesweeper game, this is random enough. no need for tech bros to swarm you
    // about their crypto secure random number generator
    srand(time(NULL)); // NOLINT(*-msc51-cpp)

    size_t minesToPlace = minefield->numMines;
    const int firstIndex = firstY * minefield->width + firstX;
    while (minesToPlace > 0) {
        const size_t randomIndex = rand() % minefield->tileCount; // NOLINT(*-msc50-cpp)

        if (randomIndex == firstIndex)
            continue;

        if (!minefield->tiles[randomIndex].isMine) {
            minefield->tiles[randomIndex].isMine = true;
            --minesToPlace;
        }
    }
}

uint8_t inline ms_234564334534_minesNearTile(const MS_Minefield *minefield, const int xPos, const int yPos) {
    if (minefield->tiles[yPos * minefield->width + xPos].isMine)
        return 0;

    uint8_t mines = 0;
    for (int y = yPos - 1; y <= yPos + 1; ++y) {
        for (int x = xPos - 1; x <= xPos + 1; ++x) {
            if (x == xPos && y == yPos)
                continue;

            if (ms_234564334534_isInsideRange(minefield, x, y)) {
                if (minefield->tiles[y * minefield->width + x].isMine) {
                    ++mines;
                }
            }
        }
    }
    return mines;
}

void ms_234564334534_countAdjacentMines(const MS_Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        minefield->tiles[i].adjacentMines = ms_234564334534_minesNearTile(minefield, i % minefield->width, i / minefield->width);
    }
}

bool ms_234564334534_openNearbyTiles(const MS_Minefield *minefield, const int xPos, const int yPos) {
    ms_234564334534_PointVector tilesToOpen;
    if (!ms_234564334534_PointVector_create(&tilesToOpen, (minefield->tileCount) >> 2)) {
        return false;
    }

    ms_234564334534_IntSet visitedTiles;
    if (!ms_234564334534_IntSet_create(&visitedTiles, (minefield->tileCount) >> 2)) {
        ms_234564334534_PointVector_destroy(&tilesToOpen);
        return false;
    }

    if (!ms_234564334534_PointVector_push(&tilesToOpen, (MS_Point){xPos, yPos})) {
        ms_234564334534_PointVector_destroy(&tilesToOpen);
        ms_234564334534_IntSet_destroy(&visitedTiles);
        return false;
    }

    if (!ms_234564334534_IntSet_insert(&visitedTiles, yPos * minefield->width + xPos)) {
        ms_234564334534_PointVector_destroy(&tilesToOpen);
        ms_234564334534_IntSet_destroy(&visitedTiles);
        return false;
    }

    while (!ms_234564334534_PointVector_is_empty(&tilesToOpen)) {
        MS_Point MS_TilePos;
        ms_234564334534_PointVector_pop(&tilesToOpen, &MS_TilePos);

        MS_Tile *currentMS_Tile = &minefield->tiles[MS_TilePos.y * minefield->width + MS_TilePos.x];
        if (currentMS_Tile->isOpen || currentMS_Tile->isFlagged || currentMS_Tile->isMine) continue;

        currentMS_Tile->isOpen = true;
        if (currentMS_Tile->adjacentMines > 0) continue;

        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                if (x == 0 && y == 0) continue;
                const int neighbourX = MS_TilePos.x + x;
                const int neighbourY = MS_TilePos.y + y;
                if (!ms_234564334534_isInsideRange(minefield, neighbourX, neighbourY))
                    continue;
                if (ms_234564334534_IntSet_contains(&visitedTiles, neighbourY * minefield->width + neighbourX)) continue;

                if (!ms_234564334534_PointVector_push(&tilesToOpen, (MS_Point){neighbourX, neighbourY})) {
                    ms_234564334534_PointVector_destroy(&tilesToOpen);
                    ms_234564334534_IntSet_destroy(&visitedTiles);
                    return false;
                }
                if (!ms_234564334534_IntSet_insert(&visitedTiles, neighbourY * minefield->width + neighbourX)) {
                    ms_234564334534_PointVector_destroy(&tilesToOpen);
                    ms_234564334534_IntSet_destroy(&visitedTiles);
                    return false;
                }
            }
        }
    }

    ms_234564334534_PointVector_destroy(&tilesToOpen);
    ms_234564334534_IntSet_destroy(&visitedTiles);

    return true;
}

void ms_234564334534_openAllMines(const MS_Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        if (minefield->tiles[i].isMine && !minefield->tiles[i].isFlagged) {
            minefield->tiles[i].isOpen = true;
        }
    }
}

void ms_234564334534_checkWinCondition(MS_Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        const MS_Tile *MS_Tile = &minefield->tiles[i];
        if (!MS_Tile->isMine && !MS_Tile->isOpen) {
            return;
        }
    }

    ms_234564334534_openAllMines(minefield);
    minefield->state = MINESWEEPER_STATE_WON;
}

bool MS_MinefieldCreate(MS_Minefield *minefield, const int width, const int height, const int numMines) {
#ifndef ndebug
    assert(minefield != NULL && "Minefield cannot be null");
#else
    if (!minefield) {
        return false;
    }
#endif

    minefield->tileCount = 0;
    minefield->tiles = NULL;
    return MS_MinefieldReset(minefield, width, height, numMines);
}

bool MS_MinefieldReset(MS_Minefield *minefield, const int width, const int height, const int numMines) {
#ifndef ndebug
    assert(minefield != NULL && "Minefield cannot be null");
    assert(width > 1 && "Width must be greater than 1");
    assert(height > 1 && "Height must be greater than 1");
    assert(numMines > 0 && "Number of mines must be greater than 0");
#else
    if (!minefield) {
        return false;
    }
#endif

    if (width < 2 || height < 2 || numMines < 1) {
        return false;
    }

    const int tileCount = width * height;

    if (tileCount != minefield->tileCount) {
        MS_Tile *reallocatedTiles = realloc(minefield->tiles, tileCount * sizeof(MS_Tile));
        if (!reallocatedTiles) {
            return false;
        }
        minefield->tiles = reallocatedTiles;
    }

    MS_Tile *tiles = minefield->tiles;
    memset(minefield, 0, sizeof(MS_Minefield));

    minefield->tiles = tiles;
    minefield->tileCount = tileCount;
    minefield->width = width;
    minefield->height = height;
    minefield->numMines = ms_234564334534_clamp(numMines, 1, tileCount >> 1);
    minefield->firstOpen = true;
    minefield->explosionPos = (MS_Point){-1, -1};
    minefield->state = MINESWEEPER_STATE_PLAYING;

    memset(minefield->tiles, 0, tileCount * sizeof(MS_Tile));

    return true;
}

void MS_MinefieldDestroy(MS_Minefield *minefield) {
    if (minefield != NULL) {
        free(minefield->tiles);
        minefield->tiles = NULL;
        minefield->width = 0;
        minefield->height = 0;
        minefield->tileCount = 0;
        minefield->numMines = 0;
        minefield->state = MINESWEEPER_STATE_ALLOC_ERROR;
        minefield->firstOpen = false;
        minefield->explosionPos = (MS_Point){-1, -1};
    }
}

MS_GameState MS_MinefieldOpenTile(MS_Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");
    if (minefield->state != MINESWEEPER_STATE_PLAYING) return minefield->state;

    if (minefield->firstOpen) {
        ms_234564334534_placeMines(minefield, xPos, yPos);
        ms_234564334534_countAdjacentMines(minefield);
        minefield->firstOpen = false;
    }

    MS_Tile *MS_Tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (MS_Tile->isFlagged || MS_Tile->isOpen) return minefield->state;

    if (MS_Tile->isMine) {
        MS_Tile->isOpen = true;
        MS_Tile->isQuestionMarked = false;
        minefield->explosionPos.x = xPos;
        minefield->explosionPos.y = yPos;
        ms_234564334534_openAllMines(minefield);
        minefield->state = MINESWEEPER_STATE_LOST;
    } else {
        if (!ms_234564334534_openNearbyTiles(minefield, xPos, yPos)) {
            minefield->state = MINESWEEPER_STATE_ALLOC_ERROR;
            return MINESWEEPER_STATE_ALLOC_ERROR;
        }
        ms_234564334534_checkWinCondition(minefield);
    }

    return minefield->state;
}

void MS_MinefieldToggleFlag(const MS_Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");

    if (minefield->state != MINESWEEPER_STATE_PLAYING)
        return;

    MS_Tile *MS_Tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (MS_Tile->isOpen)
        return;

    if (!MS_Tile->isFlagged && !MS_Tile->isQuestionMarked) {
        MS_Tile->isFlagged = true;
    } else if (MS_Tile->isFlagged) {
        MS_Tile->isFlagged = false;
        MS_Tile->isQuestionMarked = true;
    } else {
        MS_Tile->isQuestionMarked = false;
    }
}

MS_TileSprite MS_MinefieldGetTileSprite(const MS_Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");

    const MS_Tile *MS_Tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (MS_Tile->isOpen) {
        if (MS_Tile->isMine) {
            if (minefield->explosionPos.x == xPos && minefield->explosionPos.y == yPos) {
                return MS_TILE_SPRITE_EXPLODED_MINE;
            }
            return MS_TILE_SPRITE_MINE;
        }
        if (MS_Tile->adjacentMines > 0) {
            switch (MS_Tile->adjacentMines) {
                case 1: return MS_TILE_SPRITE_NUMBER_ONE;
                case 2: return MS_TILE_SPRITE_NUMBER_TWO;
                case 3: return MS_TILE_SPRITE_NUMBER_THREE;
                case 4: return MS_TILE_SPRITE_NUMBER_FOUR;
                case 5: return MS_TILE_SPRITE_NUMBER_FIVE;
                case 6: return MS_TILE_SPRITE_NUMBER_SIX;
                case 7: return MS_TILE_SPRITE_NUMBER_SEVEN;
                case 8: return MS_TILE_SPRITE_NUMBER_EIGHT;
                default: return MS_TILE_SPRITE_OPEN;
            }
        }
        if (MS_Tile->isQuestionMarked) {
            return MS_TILE_SPRITE_OPEN_QUESTION_MARK;
        }
        return MS_TILE_SPRITE_OPEN;
    }
    if (MS_Tile->isFlagged) {
        if (minefield->state == MINESWEEPER_STATE_LOST && !MS_Tile->isMine) {
            return MS_TILE_SPRITE_FALSE_MINE;
        }
        return MS_TILE_SPRITE_FLAGGED;
    }
    if (MS_Tile->isQuestionMarked) {
        return MS_TILE_SPRITE_CLOSED_QUESTION_MARK;
    }

    return MS_TILE_SPRITE_CLOSED;
}

MS_TileSprite MS_MinefieldGetTileSpriteIndex(const MS_Minefield *minefield, const int index) {
    return MS_MinefieldGetTileSprite(minefield, index % minefield->width, index / minefield->width);
}

bool MS_MinefieldWithinField(const MS_Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");

    return xPos >= 0 && xPos < minefield->width && yPos >= 0 && yPos < minefield->height;
}

const MS_Tile *MS_MinefieldGetTileFromIndex(const MS_Minefield *minefield, const int index) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(index >= 0 && index < minefield->tileCount);
    return &minefield->tiles[index];
}

bool MS_MinefieldIsGameOver(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->state == MINESWEEPER_STATE_LOST;
}

bool MS_MinefieldIsWin(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->state == MINESWEEPER_STATE_WON;
}

MS_Point MS_MinefieldGetExplosionPoint(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->explosionPos;
}

MS_Point MS_MinefieldGetTilePosition(const MS_Minefield *minefield, const int tileIndex) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(tileIndex >= 0 && tileIndex < minefield->tileCount);
    return (MS_Point){tileIndex % minefield->width, tileIndex / minefield->width};
}

int MS_MinefieldGetTileIndex(const MS_Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(ms_234564334534_isInsideRange(minefield, xPos, yPos) && "Tile position is out of range");
    return yPos * minefield->width + xPos;
}

int MS_MinefieldGetWidth(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->width;
}

int MS_MinefieldGetHeight(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->height;
}

int MS_MinefieldGetTileCount(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->tileCount;
}

int MS_MinefieldGetMineCount(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->numMines;
}

MS_GameState MS_MinefieldGetGameState(const MS_Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->state;
}
