# minesweeper-lib

A small C99 library that handles Minesweeper game logic. Build it static or shared, drop it into whatever front-end you want: CLI, GUI, game engine, doesn't matter.

It only does the game logic part: minefield generation, opening/flagging tiles, win/loss detection, and telling you what sprite each tile should show. Input handling, rendering, windowing, none of that is here. That's on you.

## Requirements

- C99 compiler (MSVC, GCC, Clang, whatever you've got)
- CMake 3.16+

## Build

```sh
mkdir build && cd build
cmake ..
cmake --build .
```

That gets you a static lib by default. If you want it shared instead (a DLL on Windows), pass `-DMINESWEEPER_BUILD_SHARED=ON`, or just set `BUILD_SHARED_LIBS` if your project already uses that convention; the option falls back to it.

```sh
cmake -DMINESWEEPER_BUILD_SHARED=ON ..
```

## Adding it to your project

Easiest way is `add_subdirectory`:

```cmake
add_subdirectory(minesweeper-lib EXCLUDE_FROM_ALL)
target_link_libraries(your_target PRIVATE minesweeper-lib::minesweeper-lib)
```

Then just pull in the one header that includes everything else:

```c
#include "MS/Minesweeper.h"
```

## Layout

```text
.
├── CMakeLists.txt
├── inc/
│   └── MS/
│       ├── Minesweeper.h       # pulls in all the headers below
│       ├── MS_Minefield.h      # the Minefield struct + the whole API
│       ├── MS_GameState.h
│       ├── MS_Tile.h
│       ├── MS_TileSprite.h
│       └── MS_Point.h
└── src/
    └── MS_Minefield.c
```

## How a tile is stored

Each tile is packed into a single byte using bit fields:

```c
typedef struct MS_Tile {
    bool    isOpen          : 1;
    bool    isMine          : 1;
    bool    isFlagged       : 1;
    bool    isQuestionMarked: 1;
    uint8_t adjacentMines   : 4;
} MS_Tile;
```

So the whole board is just `width * height` bytes, nothing fancier. A 30×16 Expert board, the biggest standard size, is 480 bytes total for every tile on the field.

## Mines aren't placed up front

`MS_MinefieldCreate` just sets up an empty board. Mines only get placed the first time you call `MS_MinefieldOpenTile`, which is also why that first click is always guaranteed safe, same as the classic Windows Minesweeper behavior.

```c
MS_Minefield mf;

if (!MS_MinefieldCreate(&mf, 9, 9, 10)) {
    fprintf(stderr, "Failed to create minefield\n");
    return 1;
}

MS_GameState state = MS_MinefieldOpenTile(&mf, 4, 4); // mines get placed right here
```

## Game state and the one error case you need to handle

`MS_GameState` has four values:

- `MINESWEEPER_STATE_PLAYING`: still going
- `MINESWEEPER_STATE_WON`: every non-mine tile is open (flagged ones count too)
- `MINESWEEPER_STATE_LOST`: stepped on a mine
- `MINESWEEPER_STATE_ALLOC_ERROR`: something failed to allocate mid-move

That last one only ever comes out of `MS_MinefieldOpenTile`. Here's why it exists: opening a tile on an empty patch of board triggers a flood-fill to clear out all the connected zeros, and that flood-fill needs two scratch buffers along the way:

- A work stack of `(x, y)` pairs to visit (8 bytes each on 64-bit), starting at `tileCount / 4` entries.
- A visited set, implemented as a hash set over tile indices: 4 bytes per entry for the index plus 1 byte for its state, also starting at `tileCount / 4` and doubling once it's 70% full, up to `tileCount` entries worst case.

Both get freed before the function returns either way. On a 30×16 board that's about 960 bytes for the stack and 600 for the set to start, call it 1.5 KB, and it only grows if the empty area you opened is huge.

If either allocation fails partway through, you can end up with some tiles that should've opened still closed, and the library flips into the error state. Nothing's corrupted, but `MS_MinefieldOpenTile` will just bail out and do nothing on subsequent calls until you reset or destroy the field. That's really the only way out of it.

```c
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
        fprintf(stderr, "Out of memory - resetting.\n");
        if (!MS_MinefieldReset(&mf, width, height, mines))
            MS_MinefieldDestroy(&mf); // reset failed too, nothing left to do
        break;
}
```

Worth noting: `MS_MinefieldCreate` and `MS_MinefieldReset` don't use this enum at all, they just return `bool`, so a plain `false` means "didn't work."

## Rendering: two ways to go about it

### Just query it every frame

For normal board sizes this is plenty fast, and it's way less code:

```c
void renderBoard(const MS_Minefield *mf) {
    int width  = MS_MinefieldGetWidth(mf);
    int height = MS_MinefieldGetHeight(mf);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MS_TileSprite sprite = MS_MinefieldGetTileSprite(mf, x, y);
            // hand sprite off to your draw call
        }
    }
}
```

### Or cache it and only refresh when something actually changes

If you'd rather not re-query the whole board every frame, allocate a sprite array once and only touch it after calls that actually mutate the field: `MS_MinefieldCreate`, `MS_MinefieldReset`, `MS_MinefieldOpenTile`, `MS_MinefieldToggleFlag`.

```c
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

    updateSpriteCache(&mf, sprites); // fill it right after creation

    MS_GameState state = MS_MinefieldOpenTile(&mf, 4, 4);
    updateSpriteCache(&mf, sprites);

    MS_MinefieldToggleFlag(&mf, 0, 0);
    updateSpriteCache(&mf, sprites);

    // sprites[] is ready for your renderer at this point

    // heads up: tileCount can change after a reset, so resize if it does
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

If you're working from the cache and need actual coordinates back, `MS_MinefieldGetTilePosition` converts a linear index to `(x, y)`:

```c
for (int i = 0; i < tileCount; ++i) {
    MS_Point pos = MS_MinefieldGetTilePosition(&mf, i);
    drawTile(pos.x, pos.y, sprites[i]);
}
```

## Putting it together

```c
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

## Full API

| Function | What it does |
|---|---|
| `MS_MinefieldCreate` | Set up a minefield and allocate its tiles |
| `MS_MinefieldReset` | Reuse the same object for a fresh game |
| `MS_MinefieldDestroy` | Free the tile memory |
| `MS_MinefieldOpenTile` | Open a tile, get back the new `MS_GameState` |
| `MS_MinefieldToggleFlag` | Cycle a tile: none → flag → question mark → none |
| `MS_MinefieldGetTileSprite` | Sprite for a tile by `(x, y)` |
| `MS_MinefieldGetTileSpriteIndex` | Sprite for a tile by linear index |
| `MS_MinefieldGetGameState` | Current `MS_GameState` |
| `MS_MinefieldIsGameOver` | True if you lost |
| `MS_MinefieldIsWin` | True if you won |
| `MS_MinefieldGetExplosionPoint` | Where the game-ending mine was |
| `MS_MinefieldWithinField` | Is `(x, y)` actually on the board |
| `MS_MinefieldGetWidth` | Board width |
| `MS_MinefieldGetHeight` | Board height |
| `MS_MinefieldGetTileCount` | `width * height` |
| `MS_MinefieldGetMineCount` | How many mines |
| `MS_MinefieldGetTilePosition` | Linear index → `(x, y)` |
| `MS_MinefieldGetTileIndex` | `(x, y)` → linear index |
