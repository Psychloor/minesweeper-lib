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
│       ├── MS_Minefield.h      # opaque MS_Minefield typedef + the whole API
│       ├── MS_GameState.h
│       ├── MS_Tile.h
│       ├── MS_TileSprite.h
│       └── MS_Point.h
└── src/
    └── MS_Minefield.c
```

## Opaque handle

`MS_Minefield` is an opaque type. The header only forward-declares it, and the actual struct lives in `MS_Minefield.c`, so you only ever work with `MS_Minefield *` and never poke at the fields directly. Layout can change between versions without breaking anyone who linked against an older build.

This means you always create and destroy through the API. No stack-allocating it, no embedding it by value in your own structs.

```c
MS_Minefield *mf = MS_MinefieldCreate(9, 9, 10);
if (!mf) {
    fprintf(stderr, "Failed to create minefield\n");
    return 1;
}

// ... use it ...

MS_MinefieldDestroy(mf);
```

`MS_MinefieldDestroy` accepts `NULL` and does nothing in that case, same as `free`, so you can call it in cleanup paths without checking first.

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
MS_Minefield *mf = MS_MinefieldCreate(9, 9, 10);
if (!mf) {
    fprintf(stderr, "Failed to create minefield\n");
    return 1;
}

MS_GameState state = MS_MinefieldOpenTile(mf, 4, 4); // mines get placed right here
```

## Game state

`MS_GameState` has three values:

- `MINESWEEPER_STATE_PLAYING`: still going
- `MINESWEEPER_STATE_WON`: every non-mine tile is open (flagged ones count too)
- `MINESWEEPER_STATE_LOST`: stepped on a mine

That's the whole enum. `MS_MinefieldOpenTile` returns one of those, and there's no allocation failure case to worry about, because the flood-fill scratch buffers live inside the minefield itself and are sized at create/reset time. More on that below.

```c
MS_GameState state = MS_MinefieldOpenTile(mf, x, y);

switch (state) {
    case MINESWEEPER_STATE_PLAYING:
        break;
    case MINESWEEPER_STATE_WON:
        printf("You won!\n");
        break;
    case MINESWEEPER_STATE_LOST:
        printf("You lost!\n");
        break;
}
```

`MS_MinefieldCreate` returns `NULL` on failure and `MS_MinefieldReset` returns `bool`, so check those at the boundary. Once you've got a valid minefield, the per-move API can't fail on you.

## Memory layout

The minefield owns three buffers, all sized to the board:

- **Tile array**: 1 byte per tile (`width * height` bytes).
- **Flood-fill work stack**: a vector of `MS_Point` (8 bytes each on 64-bit), capacity equal to the tile count.
- **Flood-fill visited bitset**: one bit per tile, so `ceil(tileCount / 8)` bytes.

So for a 30×16 Expert board:

- Tiles: 480 bytes
- Work stack: 480 × 8 = ~3.75 KB
- Visited bitset: 60 bytes

About 4.3 KB total for everything except the bookkeeping in the struct itself. The buffers are sized once at create time and resized only if you call `MS_MinefieldReset` with new dimensions. After that, `MS_MinefieldOpenTile` reuses them across every call and never allocates.

The flood fill visits each tile at most once, so neither buffer can overflow during a fill: the work stack tops out at `tileCount` entries and the bitset's address space is exactly `tileCount` bits. No growth, no probing, no failure mode.

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
    MS_Minefield *mf = MS_MinefieldCreate(9, 9, 10);
    if (!mf) {
        fprintf(stderr, "Failed to create minefield\n");
        return 1;
    }

    int tileCount = MS_MinefieldGetTileCount(mf);
    MS_TileSprite *sprites = malloc(tileCount * sizeof(MS_TileSprite));
    if (!sprites) {
        MS_MinefieldDestroy(mf);
        return 1;
    }

    updateSpriteCache(mf, sprites); // fill it right after creation

    MS_GameState state = MS_MinefieldOpenTile(mf, 4, 4);
    updateSpriteCache(mf, sprites);

    MS_MinefieldToggleFlag(mf, 0, 0);
    updateSpriteCache(mf, sprites);

    // sprites[] is ready for your renderer at this point

    // heads up: tileCount can change after a reset, so resize if it does
    MS_MinefieldReset(mf, 16, 16, 40);
    int newCount = MS_MinefieldGetTileCount(mf);
    if (newCount != tileCount) {
        MS_TileSprite *resized = realloc(sprites, newCount * sizeof(MS_TileSprite));
        if (!resized) {
            free(sprites);
            MS_MinefieldDestroy(mf);
            return 1;
        }
        sprites = resized;
        tileCount = newCount;
    }
    updateSpriteCache(mf, sprites);

    free(sprites);
    MS_MinefieldDestroy(mf);
    return 0;
}
```

If you're working from the cache and need actual coordinates back, `MS_MinefieldGetTilePosition` converts a linear index to `(x, y)`:

```c
for (int i = 0; i < tileCount; ++i) {
    MS_Point pos = MS_MinefieldGetTilePosition(mf, i);
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
    MS_Minefield *mf = MS_MinefieldCreate(9, 9, 10);
    if (!mf) {
        fprintf(stderr, "Failed to create minefield\n");
        return 1;
    }

    MS_GameState state = MS_MinefieldOpenTile(mf, 4, 4);
    printBoard(mf);

    if (state == MINESWEEPER_STATE_PLAYING) {
        MS_MinefieldToggleFlag(mf, 0, 0);
        state = MS_MinefieldOpenTile(mf, 1, 1);
        printBoard(mf);
    }

    switch (state) {
        case MINESWEEPER_STATE_WON:
            printf("You won!\n");
            break;
        case MINESWEEPER_STATE_LOST: {
            MS_Point explosion = MS_MinefieldGetExplosionPoint(mf);
            printf("You lost! Mine at (%d, %d)\n", explosion.x, explosion.y);
            break;
        }
        default:
            break;
    }

    MS_MinefieldReset(mf, 16, 16, 40);
    MS_MinefieldDestroy(mf);
    return 0;
}
```

## Full API

| Function                         | What it does                                                                   |
|----------------------------------|--------------------------------------------------------------------------------|
| `MS_MinefieldCreate`             | Allocate and set up a minefield, returns `MS_Minefield *` or `NULL` on failure |
| `MS_MinefieldReset`              | Reuse an existing minefield for a fresh game                                   |
| `MS_MinefieldDestroy`            | Free the minefield and everything it owns, safe to call on `NULL`              |
| `MS_MinefieldOpenTile`           | Open a tile, get back the new `MS_GameState`                                   |
| `MS_MinefieldToggleFlag`         | Cycle a tile: none → flag → question mark → none                               |
| `MS_MinefieldGetTileSprite`      | Sprite for a tile by `(x, y)`                                                  |
| `MS_MinefieldGetTileSpriteIndex` | Sprite for a tile by linear index                                              |
| `MS_MinefieldGetGameState`       | Current `MS_GameState`                                                         |
| `MS_MinefieldIsGameOver`         | True if you lost                                                               |
| `MS_MinefieldIsWin`              | True if you won                                                                |
| `MS_MinefieldGetExplosionPoint`  | Where the game-ending mine was                                                 |
| `MS_MinefieldWithinField`        | Is `(x, y)` actually on the board                                              |
| `MS_MinefieldGetWidth`           | Board width                                                                    |
| `MS_MinefieldGetHeight`          | Board height                                                                   |
| `MS_MinefieldGetTileCount`       | `width * height`                                                               |
| `MS_MinefieldGetMineCount`       | How many mines                                                                 |
| `MS_MinefieldGetTilePosition`    | Linear index → `(x, y)`                                                        |
| `MS_MinefieldGetTileIndex`       | `(x, y)` → linear index                                                        |