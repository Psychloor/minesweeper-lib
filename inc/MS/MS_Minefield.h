#ifndef MINESWEEPER_LIB_MINEFIELD_H
#define MINESWEEPER_LIB_MINEFIELD_H

#include "MS/MS_GameState.h"
#include "MS/MS_Point.h"
#include "MS/MS_Tile.h"
#include "MS/MS_TileSprite.h"
#include "MS/MS_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct MS_Minefield
 * @brief Represents the grid-based minefield in the Minesweeper game.
 *
 * A Minefield object is intended to be owned directly by the caller and can
 * safely be stored on the stack:
 *
 * @code
 * MS_Minefield minefield;
 * if (!MS_MinefieldCreate(&minefield, 16, 16, 40)) {
 *     // handle allocation failure
 * }
 *
 * // use minefield...
 *
 * MS_MinefieldDestroy(&minefield);
 * @endcode
 *
 * The Minefield structure itself does not need to be dynamically allocated.
 * MS_MinefieldCreate initializes the structure and allocates the internal tile
 * array. MS_MinefieldDestroy releases that internal tile memory when the caller
 * is done with the minefield.
 *
 * @note The struct definition is exposed primarily so MS_Minefield has a known
 * size to user code and can be stack allocated. Treat the fields as implementation
 * details where possible. Prefer the MS_Minefield* API functions for creating,
 * resetting, modifying, querying, and destroying the minefield.
 */
typedef struct MS_Minefield {
    MS_Tile *tiles;
    int width;
    int height;
    int tileCount;
    int numMines;
    MS_GameState state;
    int firstOpen;
    MS_Point explosionPos;
} MS_Minefield;

/**
 * @brief Initializes and creates a minefield with specified dimensions and number of mines.
 *
 * This function sets up a minefield by initializing its dimensions and the number
 * of mines and resets its internal states. It ensures the minefield pointer is valid
 * and that the necessary initializations for the tiles are carried out.
 *
 * @param minefield A pointer to the Minefield structure to be initialized. Must not be null.
 * @param width The width of the minefield in tiles.
 * @param height The height of the minefield in tiles.
 * @param numMines The number of mines to be placed within the minefield.
 * @return Returns true if the minefield was successfully created and initialized,
 *         false if the input minefield pointer is null or initialization fails.
 */
MS_EXPORT bool MS_MinefieldCreate(MS_Minefield *minefield, int width, int height, int numMines);

/**
 * @brief Resets and initializes the minefield with the specified parameters.
 *
 * This function reconfigures the given minefield structure based on the provided
 * width, height, and number of mines. It reallocates memory for the tiles
 * if necessary, clears the minefield state, and sets all relevant fields
 * to prepare the minefield for a new game session.
 *
 * The number of mines is clamped to ensure it does not exceed half the total number
 * of tiles. The function initializes the minefield to the "playing" state with no
 * tiles revealed and the explosion position unset.
 *
 * @param minefield A pointer to the Minefield structure to reset. Must not be NULL.
 * @param width The width of the minefield grid. Must be greater than 1.
 * @param height The height of the minefield grid. Must be greater than 1.
 * @param numMines The number of mines to place in the minefield. Must be greater than 0.
 * @return A boolean value indicating whether the reset operation was successful.
 *         Returns false if memory reallocation fails or if any invalid parameters are provided.
 */
MS_EXPORT bool MS_MinefieldReset(MS_Minefield *minefield, int width, int height, int numMines);


/**
 * @brief Destroys the Minefield object and releases allocated resources.
 *
 * The function deallocates the memory used for the internal `tiles` array of the
 * given minefield and resets its properties to default values, indicating a
 * non-initialized or error state. This function should be called when the
 * minefield is no longer needed to avoid memory leaks.
 *
 * @param minefield Pointer to an MS_Minefield object. If the pointer is NULL,
 *                  the function does nothing.
 */
MS_EXPORT void MS_MinefieldDestroy(MS_Minefield *minefield);

/**
 * @brief Handles the process of opening a tile in the Minesweeper game grid.
 *
 * This function manages the logic for opening a tile at the specified position in the minefield.
 * It ensures that the game rules are followed, including initializing mines on the first open,
 * handling flagged or already opened tiles, checking for mines, and validating win/loss conditions.
 *
 * @param minefield Pointer to the Minefield structure representing the game state. Must not be null.
 * @param xPos X-coordinate of the tile to open. Must be within the bounds of the minefield width.
 * @param yPos Y-coordinate of the tile to open. Must be within the bounds of the minefield height.
 * @return The updated state of the game after attempting to open the tile. Possible values are:
 *         - MINESWEEPER_STATE_PLAYING: The game is still ongoing after the operation.
 *         - MINESWEEPER_STATE_LOST: A mine was uncovered, and the game is lost.
 *         - MINESWEEPER_STATE_WON: All non-mine tiles have been successfully opened, and the game is won.
 *         - MINESWEEPER_STATE_ALLOC_ERROR: An error occurred due to memory allocation failure during the operation.
 */
MS_EXPORT MS_GameState MS_MinefieldOpenTile(MS_Minefield *minefield, int xPos, int yPos);

/**
 * @brief Toggles the flag or question mark state of a tile in the minefield.
 *
 * This function modifies the state of a specific tile in the minefield grid.
 * Depending on the current state of the tile, it alternates between the following:
 * - Unmarked to flag.
 * - Flagged to question marked.
 * - Question marked to unmarked.
 *
 * The function ensures that flags and question marks are only toggled on unopened tiles.
 * If the minefield is not in the `MINESWEEPER_STATE_PLAYING` state or the tile is already opened,
 * the function does not perform any changes.
 *
 * @param minefield A pointer to the Minefield structure that represents the minefield grid.
 *                  Must not be null.
 * @param xPos The x-coordinate of the tile to modify. Must be within the valid range
 *             [0, minefield->width - 1].
 * @param yPos The y-coordinate of the tile to modify. Must be within the valid range
 *             [0, minefield->height - 1].
 */
MS_EXPORT void MS_MinefieldToggleFlag(const MS_Minefield *minefield, int xPos, int yPos);

/**
 * @brief Retrieves the appropriate sprite for a specific tile in the minefield.
 *
 * This function determines the visual representation (sprite) of a tile in a Minesweeper game
 * based on its current state, such as whether the tile is open, flagged, a mine, or has adjacent mines.
 *
 * The tile's sprite depends on various conditions, including its openness, whether it contains a mine,
 * whether it is flagged or question-marked, and the total count of adjacent mines.
 *
 * @param minefield A pointer to the Minefield structure containing the tiles and game state.
 *                  Must not be null.
 * @param xPos The x-coordinate of the tile. Must be within the range [0, minefield->width).
 * @param yPos The y-coordinate of the tile. Must be within the range [0, minefield->height).
 * @return The sprite representing the visual state of the tile at the specified position.
 */
MS_EXPORT MS_TileSprite MS_MinefieldGetTileSprite(const MS_Minefield *minefield, int xPos, int yPos);

/**
 * @brief Retrieves the tile sprite for a given linear index in the minefield.
 *
 * This method converts a linear index into the corresponding x and y coordinates
 * within the minefield grid, then retrieves the tile sprite at that location.
 * It is useful for iterating through the minefield in a linear fashion while preserving
 * the coordinate-based structure of the minefield.
 *
 * @param minefield Pointer to the `MS_Minefield` structure representing the minefield.
 * @param index The linear index of the tile within the minefield. It is expected to
 *               be in the range [0, width * height - 1].
 * @return The tile sprite at the specified index, represented as an `MS_TileSprite` enum.
 */
MS_EXPORT MS_TileSprite MS_MinefieldGetTileSpriteIndex(const MS_Minefield *minefield, int index);

/**
 * @brief Determines if a given position is within the boundaries of a minefield.
 *
 * This function checks whether the specified x and y coordinates are valid
 * within the dimensions of the provided minefield. It ensures that the coordinates
 * are non-negative and fall within the width and height of the minefield.
 *
 * @param minefield A pointer to the Minefield structure, which represents the minefield grid.
 *                  This must not be NULL.
 * @param xPos The x-coordinate of the position to check.
 * @param yPos The y-coordinate of the position to check.
 * @return true if the specified position is within the boundaries of the minefield; otherwise, false.
 */
MS_EXPORT bool MS_MinefieldWithinField(const MS_Minefield *minefield, int xPos, int yPos);

/**
 * @brief Determines if the game is over based on the state of the minefield.
 *
 * This function checks whether the minefield's game state has reached a
 * condition where the player has lost the game. The function assumes the
 * minefield pointer is valid and asserts against null input.
 *
 * @param minefield A pointer to the Minefield structure whose state is to be evaluated.
 *                  Must not be null.
 * @return true if the game state indicates the player has lost; false otherwise.
 */
MS_EXPORT bool MS_MinefieldIsGameOver(const MS_Minefield *minefield);

/**
 * @brief Checks if the given minefield is in a winning state.
 *
 * This function evaluates whether the specified Minefield instance
 * has reached the state that indicates a win in the Minesweeper game.
 * It verifies the internal state of the minefield and determines
 * if the game has been won.
 *
 * @param minefield A pointer to the Minefield object to be checked.
 *                  Must not be null. Behavior is undefined if this
 *                  condition is violated.
 * @return True if the minefield is in a winning state, false otherwise.
 */
MS_EXPORT bool MS_MinefieldIsWin(const MS_Minefield *minefield);

/**
 * @brief Retrieves the explosion point from a given minefield.
 *
 * This function accesses the stored explosion point within the provided
 * minefield structure. The explosion point is represented as an MS_Point
 * structure, which contains the x and y coordinates where an explosion occurred.
 *
 * @param minefield A pointer to the Minefield structure from which the explosion
 * point should be retrieved. This parameter must not be NULL.
 * @return The explosion point as an MS_Point structure.
 *
 * @note The caller must ensure that the `minefield` parameter is valid and initialized.
 * If the pointer is NULL, an assertion failure will occur.
 */
MS_EXPORT MS_Point MS_MinefieldGetExplosionPoint(const MS_Minefield *minefield);

/**
 * @brief Retrieves the position of a tile within the minefield, based on its index.
 *
 * This function calculates the two-dimensional coordinates (x, y) of a specific
 * tile from its linear index in the minefield. The position is returned as an
 * MS_Point structure, where the x-coordinate represents the column, and the
 * y-coordinate represents the row.
 *
 * @param minefield Pointer to the Minefield structure representing the game board.
 *        The minefield contains the dimensions and other data related to the game board.
 * @param tileIndex The zero-based linear index of the tile for which the position
 *        is to be determined. This index corresponds to the tile's location in
 *        a flattened, one-dimensional representation of the minefield.
 * @return An MS_Point structure containing the x and y coordinates of the tile
 *         within the minefield. The x-coordinate corresponds to the column, and
 *         the y-coordinate corresponds to the row.
 */
MS_EXPORT MS_Point MS_MinefieldGetTilePosition(const MS_Minefield *minefield, int tileIndex);

/**
 * @brief Calculates the tile index in a minefield based on the x and y coordinates.
 *
 * This function computes the linear index of a tile within a minefield grid
 * using the provided x and y coordinates.
 *
 * @param minefield A pointer to the Minefield structure representing the minefield grid.
 * @param xPos The x-coordinate of the tile within the minefield.
 * @param yPos The y-coordinate of the tile within the minefield.
 * @return The computed index of the tile in the minefield's one-dimensional array.
 */
MS_EXPORT int MS_MinefieldGetTileIndex(const MS_Minefield *minefield, int xPos, int yPos);

/**
 * @brief Retrieves the width of the minefield.
 *
 * This function returns the width of the minefield, measured in the number
 * of tiles along its horizontal axis.
 *
 * @param minefield A pointer to the MS_Minefield structure whose width is to be retrieved.
 *                  The minefield must be a valid, non-null instance.
 * @return The width of the minefield as an integer.
 * @note The function will trigger an assertion failure if the provided minefield pointer is null.
 */
MS_EXPORT int MS_MinefieldGetWidth(const MS_Minefield *minefield);

/**
 * @brief Retrieves the height of the minefield.
 *
 * This function returns the height of the specified minefield, which represents
 * the number of rows in the grid structure of the minefield.
 *
 * @param minefield A pointer to the MS_Minefield structure whose height is to be retrieved.
 * This parameter must not be NULL.
 *
 * @return The height of the minefield, expressed as an integer.
 * If the provided minefield is valid, this value corresponds to the total
 * number of rows in the minefield.
 */
MS_EXPORT int MS_MinefieldGetHeight(const MS_Minefield *minefield);

/**
 * @brief Retrieves the total number of tiles in the specified minefield.
 *
 * This function provides access to the total count of tiles present in the
 * given `MS_Minefield` structure. It is useful for determining the size of
 * the minefield and facilitates operations such as iterating over all tiles.
 *
 * @param minefield A pointer to the `MS_Minefield` structure whose tile count is to be retrieved.
 *                  This parameter must not be `NULL`.
 * @return The total number of tiles in the minefield.
 */
MS_EXPORT int MS_MinefieldGetTileCount(const MS_Minefield *minefield);

/**
 * @brief Retrieves the total number of mines in the specified minefield.
 *
 * This function returns the number of mines present in the given minefield.
 * The value is determined by the `numMines` field of the `MS_Minefield` structure.
 * It is useful for displaying mine count information to the player or for
 * game logic calculations.
 *
 * @param minefield A pointer to the `MS_Minefield` structure representing the game board.
 *                  This parameter must not be NULL, and the behavior is undefined if it is.
 * @return The total number of mines in the minefield as an integer.
 */
MS_EXPORT int MS_MinefieldGetMineCount(const MS_Minefield *minefield);

/**
 * @brief Retrieves the current game state of the given minefield.
 *
 * This function provides the game state of the Minesweeper game
 * based on the state of the provided minefield. The game state
 * can be one of the enumerated values in `MS_GameState`, such as
 * playing, won, and lost.
 *
 * The minefield must be a valid and non-NULL pointer; otherwise,
 * the behavior is undefined.
 *
 * @param minefield A pointer to the `MS_Minefield` structure representing
 * the current state of the Minesweeper game. It is expected to
 * be properly initialized before invoking this function.
 *
 * @return The current state of the game as an `MS_GameState` value.
 */
MS_EXPORT MS_GameState MS_MinefieldGetGameState(const MS_Minefield *minefield);

#ifdef __cplusplus
}
#endif

#endif
