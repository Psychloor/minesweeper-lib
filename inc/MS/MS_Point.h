#ifndef MINESWEEPER_LIB_MS_POINT_H
#define MINESWEEPER_LIB_MS_POINT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct MS_Point
 * @brief Represents a point in a two-dimensional coordinate system.
 *
 * This structure is used to define a coordinate represented by
 * x and y values. It is particularly useful in the context of
 * the Minesweeper game to designate positions on the minefield grid.
 *
 * It serves as a fundamental unit for operations involving positions, such as
 * navigating the minefield, identifying neighboring tiles, or specifying
 * explosion locations.
 *
 * @note This structure does not enforce any range restrictions on the x and y values.
 */
typedef struct MS_Point {
    /**
     * @brief Represents the x-coordinate of a point or position.
     *
     * This variable is commonly used to define the horizontal position of an object
     * in a coordinate system, such as within the context of a minefield or grid.
     *
     * @note It is an integral value.
     */
    int x;
    /**
     * Represents the vertical position or coordinate within a grid or minefield.
     * Typically used to reference rows in two-dimensional structures,
     * such as the layout of a Minesweeper game board.
     */
    int y;
} MS_Point;

#ifdef __cplusplus
}
#endif

#endif //MINESWEEPER_LIB_MS_POINT_H
