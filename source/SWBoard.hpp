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
    Board();
    Board(const int row, const int col);
    Board(Vec2 v);
    /**
     * Disposes the board, releasing all resources
     */
    ~Board() {}


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
     * Switches position of two squares on the board.
     *
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
    void switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2);

    /**
     * Returns the squares being attacked by a given unit on a square.
     *
     * @param pos the attacker square's position
     * @return a list of squares being attacked.
     */
    vector<Square> getVictims(cugl::Vec2 pos, bool special);

    /**
     * Returns the squares being attacked by a given unit on a square.
     *
     * @param pos the attacker square's position
     * @return a list of squares being attacked.
     */
    vector<Square> getVictims(Square square, bool special);

    /** 
     * Returns if a square exists on the board.
     * 
     * @param pos the position of the square.
     * @return whether the square exists.
    */
    bool doesSqaureExist(cugl::Vec2 pos);
};
#endif /* SWBoard_hpp */
