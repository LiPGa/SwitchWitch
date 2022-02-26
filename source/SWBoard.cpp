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

}
Board::Board(Vec2 v)
{
    row = v.x;
    col = v.y;
    matrix = vector<vector<Square>>(row, vector<Square>(col));

}

/**
 * Switch squares
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
 * Returns the squares being attacked.
 *
 * @param pos the attacker square's position
 * @return the list of the victims' position
 */
vector<Square> Board::getVictims(cugl::Vec2 pos, bool special)
{
    return getVictims(getSquare(pos), special);
}

/**
 * Returns the squares being attacked.
 *
 * @param pos the attacker square's position
 * @return the list of the victims' position
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

bool Board::doesSqaureExist(cugl::Vec2 pos)
{
    return ((pos.x >= 0) &&
            (pos.x < col) &&
            (pos.y >= 0) &&
            (pos.y < row));
}
