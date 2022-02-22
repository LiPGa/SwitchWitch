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
Board::Board(const int row, const int col) {
    _row = row;
    _col = col;
    for(int i = 0 ; i < row ; i++){
        matrix[i].resize(col);
    }
}

/**
 * Switch squares
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2) {
    Square main = getSquare(pos1.x, pos1.y);
    Square second = getSquare(pos2.x, pos2.y);
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
    Square main = getSquare(pos.x, pos.y);
    Unit::Attacks pattern = main.getUnit().getAttack();
    Unit::Directions direction = main.getUnit().getDirection();
    if(pattern == Unit::Attacks::basicAttack){
        return vector<cugl::Vec2>();
    }
    if(pattern == Unit::Attacks::doubleAttack) {
        if(direction == Unit::Directions::left) {
            if(pos.x-2 >= 0) {
                list.push_back(Vec2 (pos.x-2, pos.y));
            }
            if(pos.x - 1 >= 0) {
                list.push_back(Vec2 (pos.x-1, pos.y));
            }
        }
        if(direction == Unit::Directions::right) {
            if(pos.x+2 <= _col) {
                list.push_back(Vec2 (pos.x+2, pos.y));
            }
            if(pos.x+1 <= _col) {
                list.push_back(Vec2 (pos.x+1, pos.y));
            }
        }
        if(direction == Unit::Directions::up) {
            if(pos.y+2 <= _row) {
                list.push_back(Vec2 (pos.x, pos.y+2));
            }
            if(pos.y+1 <= _row) {
                list.push_back(Vec2 (pos.x, pos.y+1));
            }
        }
        if(direction == Unit::Directions::down) {
            if(pos.y-2 >= 0) {
                list.push_back(Vec2 (pos.x, pos.y-2));
            }
            if(pos.y-1 >= 0) {
                list.push_back(Vec2 (pos.x, pos.y-1));
            }
        }
    }
    if(pattern == Unit::Attacks::tripleAttack) {
        if(direction == Unit::Directions::left) {
            if(pos.x - 1 >= 0 && pos.y -1 >=0){
                list.push_back(Vec2 (pos.x -1, pos.y-1));
            }
            if(pos.x - 1 >= 0){
                list.push_back(Vec2 (pos.x -1, pos.y));
            }
            if(pos.x - 1 >= 0 && pos.y +1 <=_row){
                list.push_back(Vec2 (pos.x -1, pos.y+1));
            }
        }
        if(direction == Unit::Directions::right) {
            if(pos.x + 1 <= _col && pos.y -1 >=0){
                list.push_back(Vec2 (pos.x +1, pos.y-1));
            }
            if(pos.x + 1 <= _col){
                list.push_back(Vec2 (pos.x +1, pos.y));
            }
            if(pos.x + 1 <= _col && pos.y +1 <=_row){
                list.push_back(Vec2 (pos.x +1, pos.y+1));
            }
        }
        if(direction == Unit::Directions::up) {
            if(pos.x - 1 >= 0 && pos.y - 1 >= 0){
                list.push_back(Vec2 (pos.x-1, pos.y-1));
            }
            if(pos.y - 1 >= 0){
                list.push_back(Vec2 (pos.x, pos.y-1));
            }
            if(pos.x + 1 <= _col && pos.y -1 >= 0){
                list.push_back(Vec2 (pos.x +1, pos.y-1));
            }
        }
        if(direction == Unit::Directions::down) {
            if(pos.x - 1 >= 0 && pos.y + 1 <= _row) {
                list.push_back(Vec2 (pos.x-1, pos.y+1));
            }
            if(pos.y + 1 <=_row){
                list.push_back(Vec2 (pos.x, pos.y+1));
            }
            if(pos.x + 1 <= _col && pos.y +1 <= _row){
                list.push_back(Vec2 (pos.x +1, pos.y+1));
            }
        }
        if(pattern == Unit::Attacks::tripleAttack) {
            if (pos.x - 1 >= 0 && pos.y - 1 >= 0) {
                list.push_back(Vec2 (pos.x-1, pos.y-1));
            }
            if (pos.x - 1 >= 0 && pos.y + 1 <= _row) {
                list.push_back(Vec2 (pos.x-1, pos.y+1));
            }
            if (pos.x + 1 <= _col && pos.y - 1 >= 0) {
                list.push_back(Vec2 (pos.x+1, pos.y-1));
            }
            if (pos.x + 1 <= _col && pos.y + 1 <= _row) {
                list.push_back(Vec2 (pos.x+1, pos.y+1));
            }
        }
        
        
    }
    return list;
};
