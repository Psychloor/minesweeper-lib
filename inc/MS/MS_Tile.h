//
// Created by blomq on 2026-06-16.
//

#ifndef MINESWEEPER_LIB_TILE_H
#define MINESWEEPER_LIB_TILE_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @struct MS_Tile
 * Represents a single tile in a Minesweeper game grid.
 * Each tile contains information about its current state and surrounding mines.
 *
 * @var MS_Tile::isOpen
 * Bit flag indicating whether the tile is open. When true, the tile has been revealed.
 *
 * @var MS_Tile::isMine
 * Bit flag indicating whether the tile contains a mine. When true, the tile is a mine.
 *
 * @var MS_Tile::isFlagged
 * Bit flag indicating whether the tile is flagged. When true, the tile is marked as a potential mine.
 *
 * @var MS_Tile::isQuestionMarked
 * Bit flag indicating whether the tile is question-marked. When true, the tile is uncertain.
 *
 * @var MS_Tile::adjacentMines
 * A 4-bit value indicating the count of mines adjacent to the tile. Ranges from 0 to 8.
 */
typedef struct MS_Tile {
    /**
     * Indicates whether the tile is open in the Minesweeper game.
     *
     * This member is a 1-bit boolean value that determines if the
     * tile has been revealed by the player. If set to `true` (1),
     * the tile is open, and its underlying characteristics (such as
     * being a mine or having a specific number of adjacent mines)
     * are visible to the player. If set to `false` (0), the tile
     * remains hidden.
     *
     * Usage:
     * - A tile marked as `isOpen = true` is considered revealed.
     * - Tiles marked as `isOpen = false` are hidden from the player's view.
     */
    bool isOpen: 1;
    /**
     * @brief Represents whether a tile in the Minesweeper game contains a mine.
     *
     * When set to `true`, this indicates that the corresponding tile contains a mine.
     * When set to `false`, the tile is free of mines. This field is utilized during
     * the game's initialization and gameplay to determine mine placement and behavior.
     */
    bool isMine: 1;
    /**
     * Indicates if the tile is flagged by the player.
     *
     * A flagged tile is marked by the player to signal that they believe
     * the tile contains a mine. This does not inherently affect the
     * game's logic but prevents the flagged tile from being opened during play.
     *
     * - `true`: The tile is flagged.
     * - `false`: The tile is not flagged.
     */
    bool isFlagged: 1;
    /**
     * Indicates whether this tile is marked with a question mark.
     * This is used as a third state for tile marking in the game, in addition to being flagged or left unmarked.
     *
     * When a tile is in the "question marked" state, it visually represents uncertainty about whether the tile
     * contains a mine. It does not affect game mechanics directly, as it is intended for the player's reference only.
     *
     * Possible states:
     * - 1: The tile is marked with a question mark.
     * - 0: The tile is not marked with a question mark.
     */
    bool isQuestionMarked: 1;
    /**
     * Represents the number of mines adjacent to a specific tile in the minefield.
     *
     * The value is limited to a maximum of 4 bits (0-15) to optimally store the count.
     * This field is primarily used in gameplay logic to determine the number of
     * neighboring mines for a given tile, which is displayed to the player under
     * certain conditions (e.g., when the tile is opened and doesn't contain a mine).
     */
    uint8_t adjacentMines: 4;
} MS_Tile;

#endif //MINESWEEPER_LIB_TILE_H
