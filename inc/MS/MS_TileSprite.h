//
// Created by blomq on 2026-06-17.
//

#ifndef MINESWEEPER_LIB_MS_TILE_SPRITE_H
#define MINESWEEPER_LIB_MS_TILE_SPRITE_H

/**
 * @enum MS_TileSprite
 * @brief Represents the various states and types of tile sprites in the Minesweeper game.
 *
 * This enumeration defines the different sprite states that a tile
 * in the game can display, including numbered tiles, open and closed tiles,
 * flagged tiles, and mine states.
 *
 * - MS_TILE_SPRITE_NUMBER_ONE: Indicates a tile displaying the number one.
 * - MS_TILE_SPRITE_NUMBER_TWO: Indicates a tile displaying the number two.
 * - MS_TILE_SPRITE_NUMBER_THREE: Indicates a tile displaying the number three.
 * - MS_TILE_SPRITE_NUMBER_FOUR: Indicates a tile displaying the number four.
 * - MS_TILE_SPRITE_NUMBER_FIVE: Indicates a tile displaying the number five.
 * - MS_TILE_SPRITE_NUMBER_SIX: Indicates a tile displaying the number six.
 * - MS_TILE_SPRITE_NUMBER_SEVEN: Indicates a tile displaying the number seven.
 * - MS_TILE_SPRITE_NUMBER_EIGHT: Indicates a tile displaying the number eight.
 * - MS_TILE_SPRITE_OPEN: Represents an open tile without a number.
 * - MS_TILE_SPRITE_CLOSED: Represents a closed tile.
 * - MS_TILE_SPRITE_FLAGGED: Represents a tile marked with a flag by the player.
 * - MS_TILE_SPRITE_FALSE_MINE: Represents a tile incorrectly flagged as a mine.
 * - MS_TILE_SPRITE_OPEN_QUESTION_MARK: Represents an open tile with a question mark.
 * - MS_TILE_SPRITE_CLOSED_QUESTION_MARK: Represents a closed tile with a question mark.
 * - MS_TILE_SPRITE_MINE: Represents a tile containing a mine.
 * - MS_TILE_SPRITE_EXPLODED_MINE: Represents a tile containing a mine that has exploded.
 * - MS_TILE_SPRITE_COUNT: Represents the total number of sprite types.
 */
typedef enum MS_TileSprite {
    MS_TILE_SPRITE_NUMBER_ONE,
    MS_TILE_SPRITE_NUMBER_TWO,
    MS_TILE_SPRITE_NUMBER_THREE,
    MS_TILE_SPRITE_NUMBER_FOUR,
    MS_TILE_SPRITE_NUMBER_FIVE,
    MS_TILE_SPRITE_NUMBER_SIX,
    MS_TILE_SPRITE_NUMBER_SEVEN,
    MS_TILE_SPRITE_NUMBER_EIGHT,
    MS_TILE_SPRITE_OPEN,
    MS_TILE_SPRITE_CLOSED,
    MS_TILE_SPRITE_FLAGGED,
    MS_TILE_SPRITE_FALSE_MINE,
    MS_TILE_SPRITE_OPEN_QUESTION_MARK,
    MS_TILE_SPRITE_CLOSED_QUESTION_MARK,
    MS_TILE_SPRITE_MINE,
    MS_TILE_SPRITE_EXPLODED_MINE,
    MS_TILE_SPRITE_COUNT
} MS_TileSprite;

#endif
