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
Board::Board(const int r, const int c) {
    row = r;
    col = c;
//    for(int i = 0 ; i < row ; i++){
//        matrix[i].resize(col);
//    }
}

/**
 * Switch squares
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2) {
    Square main = getSquare(pos1);
    Square second = getSquare(pos2);
    setSquare(pos1, main);
    setSquare(pos2, second);
}

/**
 * Returns the squares being attacked.
 *
 * @param pos the attacker square's position
 * @return the list of the victims' position
 */
vector<cugl::Vec2> Board::getVictims(cugl::Vec2 pos) {
    vector<cugl::Vec2> list;
    Square main = getSquare(pos);
    vector<cugl::Vec2> pattern;
    cugl::Vec2 direction;
    pattern= main.getUnit().getAttack();
    int d = 0;
    if (pattern == Unit.down)
        d = 0;
    
    if (pattern == Unit.left)
        d = 1;
    
    if (pattern == Unit.up)
        d = 2;
    
    if (pattern == Unit.right)
        d = 3;
    
    for (auto it = pattern.begin(); it != pattern.end(); ++it){
        list.push_back(it->getRotation(d*M_PI_2));
    }
    return list;

    
}

bool Board::doesSqaureExist(cugl::Vec2 pos){
    return ((pos.x >= 0) &&
    (pos.x < col) &&
    (pos.y >= 0) &&
    (pos.y < row));
}
