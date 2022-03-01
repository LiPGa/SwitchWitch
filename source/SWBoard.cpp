//
//  SWBoard.cpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWBoard.hpp"
using namespace cugl;

/**
 * Creates a board with the given size
 *
 * @param width The width of the board
 * @param height The height of the board
 */
Board::Board(){}
Board::Board(const int r, const int c)
{
    row = r;
    col = c;
    matrix = vector<vector<Square>>(row, vector<Square>(col));
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++){
            matrix[i][j].setPosition(Vec2(i,j));
        }
    }

}
Board::Board(Vec2 v)
{
    Board(v.x, v.y);
}

/**
 * Switches position of two squares on the board.
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2)
{
    Square s = getSquare(pos1);
    Square second = getSquare(pos2);
    setSquare(pos1, s);
    setSquare(pos2, second);
}

/**
 * Switches position of two units while not moving the squares.
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchUnits(cugl::Vec2 pos1, cugl::Vec2 pos2){
    Square first = getSquare(pos1);
    Square second = getSquare(pos2);
    Unit firstUnit = first.getUnit();
    Unit secondUnit = second.getUnit();
    first.setUnit(secondUnit);
    second.setUnit(firstUnit);
}

/**
 * Returns the squares being attacked by a given unit on a square.
 *
 * @param pos the attacker square's position
 * @return a list of squares being attacked.
 */
vector<Square> Board::getVictims(cugl::Vec2 pos, bool special)
{
    return getVictims(getSquare(pos), special);
}

/**
 * Returns the squares being attacked by a given unit on a square.
 *
 * @param pos the attacker square's position
 * @return a list of squares being attacked.
 */
vector<Square> Board::getVictims(Square square, bool special)
{
    Unit selectedUnit = square.getUnit();
    float angle = cugl::Vec2::angle(selectedUnit.getDirection(), selectedUnit.defaultDirection);
    std::vector<cugl::Vec2> attacks = special ? selectedUnit.getSpecialAttack() : selectedUnit.getBasicAttack();
    std::vector<Square> result;
    for (auto it = attacks.begin(); it != attacks.end(); ++it)
    {
        result.push_back(getSquare(it->getRotation(angle)));
    }
    return result;
}

/** 
 * Returns if a square exists on the board.
 * 
 * @param pos the position of the square.
 * @return whether the square exists.
*/
bool Board::doesSqaureExist(cugl::Vec2 pos)
{
    return ((pos.x >= 0) &&
            (pos.x < col) &&
            (pos.y >= 0) &&
            (pos.y < row));
}
