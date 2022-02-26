//
//  SWBoard.hpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWBoard_hpp
#define SWBoard_hpp

#include "SWSquare.hpp"
#include <cugl/cugl.h>

class Board
{
private:
    /** Size of the board */
    int row;
    int col;
    /** Square matrix on this board*/
    vector<vector<Square>> matrix;

public:
#pragma mark Constructors
    /**
     * Creates a board with the given size
     *
     * @param width The width of the board
     * @param height The height of the board
     */
    Board(const int row, const int col);

    /**
     * Disposes the board, releasing all resources
     */
    ~Board() {}

    Board(Vec2 v);

#pragma mark Properties
    /**
     * Returns the square at a position
     *
     * This is location of the center pixel of the unit on the screen
     *
     * @param _pos the position of a square
     * @return the square  at a position
     */
    const Square &getSquare(Vec2 input) const
    {
        return matrix[input.x][input.y];
    }

    /**
     * Sets the position of this square.
     *
     * This is location of the center pixel of the square on the screen.
     *
     * @param value the position of this ship
     */
    void setSquare(cugl::Vec2 value, Square square)
    {
        matrix[value.x][value.y] = square;
    }

    /**
     * Switch squares
     *
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
    void switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2);

    /**
     * Returns the squares being attacked.
     *
     * @param pos the attacker square's position
     * @return the list of the victims' position
     */
    vector<Square> getVictims(cugl::Vec2 pos, bool special);

    /**
     * Returns the squares being attacked.
     *
     * @param pos the attacker square's position
     * @return the list of the victims' position
     */
    vector<Square> getVictims(Square square, bool special);

    /**
     Returns 1 if the square exists at the given Vec2 position
     returns 0 otherwise
     @param pos the position being questioned
     @return a bool with 1 meaning yes, 0 meaning no
     */
    bool doesSqaureExist(cugl::Vec2 pos);
};
#endif /* SWBoard_hpp */
