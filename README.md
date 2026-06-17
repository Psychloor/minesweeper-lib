# minesweeper-lib

A C99 static library providing Minesweeper game logic.

## Overview

`minesweeper-lib` handles the core mechanics of a Minesweeper game:
minefield generation, tile management, flag toggling, win/loss detection,
and rendering queries via tile sprites. It is designed to be integrated into
any front-end — CLI, GUI, game engine, etc.

The library does not handle input, rendering, or windowing. That is left
entirely to the consumer.

## Requirements

- **Language:** C99
- **Build System:** CMake 3.16 or higher
- **Compiler:** Any C99 compliant compiler (MSVC, GCC, Clang, etc.)

## Project Structure

```text
.
├── CMakeLists.txt
├── inc/
│   └── MS/
│       ├── Minesweeper.h       # Convenience header, includes everything below
│       ├── MS_Minefield.h      # Minefield struct and full API
│       ├── MS_GameState.h      # MS_GameState enum
│       ├── MS_Tile.h           # MS_Tile struct
│       ├── MS_TileSprite.h     # MS_TileSprite enum
│       └── MS_Point.h          # MS_Point struct
└── src/
    └── MS_Minefield.c
```

## Build

```sh
mkdir build && cd build
cmake ..
cmake --build .
```

Produces `minesweeper-lib.lib` (Windows) or `libminesweeper-lib.a` (Linux/macOS).

## Integrating with CMake

If you are using CMake in your own project, you can add this library as a
subdirectory:

```c++make
add_subdirectory(minesweeper-lib EXCLUDE_FROM_ALL)
target_link_libraries(your_target PRIVATE minesweeper-lib::minesweeper-lib)
```

Then include the master header:

```c++
#include "MS/Minesweeper.h"
```

## Memory and Tile Size

Each tile (`MS_Tile`) is stored as a single byte using bit fields:

```c++
typedef struct MS_Tile {
    bool    isOpen          : 1;  /* revealed                  */
    bool    isMine          : 1;  /* contains a mine           */
    bool    isFlagged       : 1;  /* flagged by player         */
    bool    isQuestionMarked: 1;  /* question-marked by player */
    uint8_t adjacentMines   : 4;  /* count 0–8                 */
} MS_Tile;                        /* sizeof(MS_Tile) == 1      */
```

The internal tile array is therefore exactly `width * height` bytes.
A 30×16 Expert board (the largest standard size) uses 480 bytes for all tiles.

## Game State and Error Handling

`MS_GameState` has four values:

| State                           | Meaning                                                 |
|---------------------------------|---------------------------------------------------------|
| `MINESWEEPER_STATE_PLAYING`     | Game is active, moves can be made                       |
| `MINESWEEPER_STATE_WON`         | All non-mine tiles opened (flagged counts), game is won |
| `MINESWEEPER_STATE_LOST`        | A mine was opened, game is over                         |
| `MINESWEEPER_STATE_ALLOC_ERROR` | An internal allocation failed mid-game                  |

`MINESWEEPER_STATE_ALLOC_ERROR` can only come from `MS_MinefieldOpenTile`.
When opening a safe tile, the library temporarily allocates two internal
buffers to flood-fill connected empty tiles, both freed before the function
returns:

- **Work stack** — a dynamic array of `(x, y)` coordinate pairs (8 bytes each
  on 64-bit). Initial capacity is `tileCount / 4` entries.
- **Visited set** — a hash set of tile indices (`int`), using two separate
  arrays: 4 bytes per entry for the index and 1 byte per entry for its state.
  Initial capacity is `tileCount / 4` entries, growing by doubling when 70%
  full, up to at most `tileCount` entries in the worst case.

On a 30×16 board (`tileCount = 480`), the initial allocation is 120 entries
each, costing roughly **960 bytes** for the stack and **600 bytes** for the
set — about **1.5 KB** combined at startup, growing only if the fill is large.
If either allocation fails mid-fill, some tiles that should have been opened
may not be, and the game is put into the error state. The tile data is not
corrupted, but further calls to `MS_MinefieldOpenTile` will return early
without doing anything. The safest recovery is to reset or destroy the
minefield.

Always check the return value of `MS_MinefieldOpenTile`:

```c++
MS_GameState state = MS_MinefieldOpenTile(&mf, x, y);

switch (state) {
    case MINESWEEPER_STATE_PLAYING:
        break;
    case MINESWEEPER_STATE_WON:
        printf("You won!\n");
        break;
    case MINESWEEPER_STATE_LOST:
        printf("You lost!\n");
        break;
    case MINESWEEPER_STATE_ALLOC_ERROR:
        fprintf(stderr, "Out of memory — resetting.\n");
        if (!MS_MinefieldReset(&mf, width, height, mines))
            MS_MinefieldDestroy(&mf); /* reset also failed */
        break;
}
```

`MS_MinefieldCreate` and `MS_MinefieldReset` return `bool` and do not use
`MINESWEEPER_STATE_ALLOC_ERROR` — they simply return `false` on failure.

## Rendering

The library provides `MS_TileSprite` values to describe what each tile should
look like. There are two approaches for rendering, depending on your needs.

### Option A: Query every frame

Query sprites directly each frame. Simple, and perfectly fast for normal
board sizes.

```c++
void renderBoard(const MS_Minefield *mf) {
    int width  = MS_MinefieldGetWidth(mf);
    int height = MS_MinefieldGetHeight(mf);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MS_TileSprite sprite = MS_MinefieldGetTileSprite(mf, x, y);
            /* pass sprite to your draw function */
        }
    }
}
```

### Option B: Cache sprites, update on mutations

Allocate a sprite array once and refresh it only after actions that change
tile state: `MS_MinefieldCreate`, `MS_MinefieldReset`, `MS_MinefieldOpenTile`,
and `MS_MinefieldToggleFlag`.

```c++
void updateSpriteCache(const MS_Minefield *mf, MS_TileSprite *cache) {
    int count = MS_MinefieldGetTileCount(mf);
    for (int i = 0; i < count; ++i) {
        cache[i] = MS_MinefieldGetTileSpriteIndex(mf, i);
    }
}

int main(void) {
    MS_Minefield mf;

    if (!MS_MinefieldCreate(&mf, 9, 9, 10)) {
        fprintf(stderr, "Failed to create minefield\n");
        return 1;
    }

    int tileCount = MS_MinefieldGetTileCount(&mf);
    MS_TileSprite *sprites = malloc(tileCount * sizeof(MS_TileSprite));
    if (!sprites) {
        MS_MinefieldDestroy(&mf);
        return 1;
    }

    /* Initial cache fill after creation */
    updateSpriteCache(&mf, sprites);

    /* Open a tile, then refresh the cache */
    MS_GameState state = MS_MinefieldOpenTile(&mf, 4, 4);
    updateSpriteCache(&mf, sprites);

    /* Flag a tile, refresh again */
    MS_MinefieldToggleFlag(&mf, 0, 0);
    updateSpriteCache(&mf, sprites);

    /* sprites[] is now ready to hand to your renderer */

    /* Reset to a new game — tileCount may change, so reallocate if needed */
    MS_MinefieldReset(&mf, 16, 16, 40);
    int newCount = MS_MinefieldGetTileCount(&mf);
    if (newCount != tileCount) {
        MS_TileSprite *resized = realloc(sprites, newCount * sizeof(MS_TileSprite));
        if (!resized) {
            free(sprites);
            MS_MinefieldDestroy(&mf);
            return 1;
        }
        sprites = resized;
        tileCount = newCount;
    }
    updateSpriteCache(&mf, sprites);

    free(sprites);
    MS_MinefieldDestroy(&mf);
    return 0;
}
```

`MS_MinefieldGetTilePosition` converts a linear index back to `(x, y)` if
your renderer needs coordinates:

```c++
for (int i = 0; i < tileCount; ++i) {
    MS_Point pos = MS_MinefieldGetTilePosition(&mf, i);
    drawTile(pos.x, pos.y, sprites[i]);
}
```

## Basic Usage Example

`MS_Minefield` can be stored on the stack. `MS_MinefieldCreate` allocates
the internal tile array. `MS_MinefieldDestroy` frees it when you are done.

Mines are not placed at creation time. They are placed on the first call to
`MS_MinefieldOpenTile`, so the first opened tile is always safe.

```c++
#include "MS/Minesweeper.h"
#include <stdio.h>

void printBoard(const MS_Minefield *mf) {
    int width  = MS_MinefieldGetWidth(mf);
    int height = MS_MinefieldGetHeight(mf);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MS_TileSprite sprite = MS_MinefieldGetTileSprite(mf, x, y);
            char c;
            switch (sprite) {
                case MS_TILE_SPRITE_CLOSED:               c = '.'; break;
                case MS_TILE_SPRITE_FLAGGED:              c = 'F'; break;
                case MS_TILE_SPRITE_CLOSED_QUESTION_MARK: c = '?'; break;
                case MS_TILE_SPRITE_OPEN:                 c = ' '; break;
                case MS_TILE_SPRITE_MINE:                 c = '*'; break;
                case MS_TILE_SPRITE_EXPLODED_MINE:        c = 'X'; break;
                case MS_TILE_SPRITE_FALSE_MINE:           c = '!'; break;
                case MS_TILE_SPRITE_NUMBER_ONE:           c = '1'; break;
                case MS_TILE_SPRITE_NUMBER_TWO:           c = '2'; break;
                case MS_TILE_SPRITE_NUMBER_THREE:         c = '3'; break;
                case MS_TILE_SPRITE_NUMBER_FOUR:          c = '4'; break;
                case MS_TILE_SPRITE_NUMBER_FIVE:          c = '5'; break;
                case MS_TILE_SPRITE_NUMBER_SIX:           c = '6'; break;
                case MS_TILE_SPRITE_NUMBER_SEVEN:         c = '7'; break;
                case MS_TILE_SPRITE_NUMBER_EIGHT:         c = '8'; break;
                default:                                  c = '?'; break;
            }
            putchar(c);
        }
        putchar('\n');
    }
}

int main(void) {
    MS_Minefield mf;

    if (!MS_MinefieldCreate(&mf, 9, 9, 10)) {
        fprintf(stderr, "Failed to create minefield\n");
        return 1;
    }

    MS_GameState state = MS_MinefieldOpenTile(&mf, 4, 4);
    printBoard(&mf);

    if (state == MINESWEEPER_STATE_PLAYING) {
        MS_MinefieldToggleFlag(&mf, 0, 0);
        state = MS_MinefieldOpenTile(&mf, 1, 1);
        printBoard(&mf);
    }

    switch (state) {
        case MINESWEEPER_STATE_WON:
            printf("You won!\n");
            break;
        case MINESWEEPER_STATE_LOST: {
            MS_Point explosion = MS_MinefieldGetExplosionPoint(&mf);
            printf("You lost! Mine at (%d, %d)\n", explosion.x, explosion.y);
            break;
        }
        case MINESWEEPER_STATE_ALLOC_ERROR:
            fprintf(stderr, "Allocation error during game\n");
            break;
        default:
            break;
    }

    MS_MinefieldReset(&mf, 16, 16, 40);
    MS_MinefieldDestroy(&mf);
    return 0;
}
```

## API Summary

| Function                         | Description                                         |
|----------------------------------|-----------------------------------------------------|
| `MS_MinefieldCreate`             | Initialize a minefield and allocate tiles           |
| `MS_MinefieldReset`              | Reset to a new game, reusing the same object        |
| `MS_MinefieldDestroy`            | Free internal tile memory                           |
| `MS_MinefieldOpenTile`           | Open a tile, returns updated `MS_GameState`         |
| `MS_MinefieldToggleFlag`         | Cycle tile mark: none → flag → question mark → none |
| `MS_MinefieldGetTileSprite`      | Get the visual state of a tile by (x, y)            |
| `MS_MinefieldGetTileSpriteIndex` | Get the visual state of a tile by linear index      |
| `MS_MinefieldGetGameState`       | Get current `MS_GameState`                          |
| `MS_MinefieldIsGameOver`         | True if the player lost                             |
| `MS_MinefieldIsWin`              | True if the player won                              |
| `MS_MinefieldGetExplosionPoint`  | Get position of the mine that ended the game        |
| `MS_MinefieldWithinField`        | Check if (x, y) is within bounds                    |
| `MS_MinefieldGetWidth`           | Board width in tiles                                |
| `MS_MinefieldGetHeight`          | Board height in tiles                               |
| `MS_MinefieldGetTileCount`       | Total tile count (width × height)                   |
| `MS_MinefieldGetMineCount`       | Number of mines                                     |
| `MS_MinefieldGetTilePosition`    | Convert linear index to (x, y)                      |
| `MS_MinefieldGetTileIndex`       | Convert (x, y) to linear index                      |
