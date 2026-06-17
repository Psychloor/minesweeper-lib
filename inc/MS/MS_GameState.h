#ifndef MINESWEEPER_LIB_MS_GAME_STATE_H
#define MINESWEEPER_LIB_MS_GAME_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum MS_GameState
 * @brief The possible states of the Minesweeper game.
 *
 * This enumeration defines the various outcomes or statuses that
 * the Minesweeper game can be in during its lifecycle.
 *
 * - MINESWEEPER_STATE_PLAYING: The game is currently active, and moves can be made.
 * - MINESWEEPER_STATE_LOST: The game has ended because a mine was triggered.
 * - MINESWEEPER_STATE_WON: The game has been successfully completed by clearing all safe tiles.
 * - MINESWEEPER_STATE_ALLOC_ERROR: A memory allocation failure occurred, preventing the game from continuing.
 *
 * This enumeration is commonly used to track and verify the state of the game at various points
 * in the application's logic.
 */
typedef enum MS_GameState {
    MINESWEEPER_STATE_PLAYING,
    MINESWEEPER_STATE_LOST,
    MINESWEEPER_STATE_WON,
    MINESWEEPER_STATE_ALLOC_ERROR
} MS_GameState;

#ifdef __cplusplus
}
#endif

#endif
