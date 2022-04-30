//
//  SWBoard.cpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWBoard.hpp"
using namespace cugl;

#pragma mark Constructors
/**
 * Initializes a board with the given size
 * NOTE: It will also allocate squares for you so that all points on the board will have a square. 
 * Units in squares are not allocated.
 * 
 * @param rows The number of rows on the board.
 * @param columns The number of columns on the board.
 */
bool Board::init(int columns, int rows) {
    _matrix = vector<shared_ptr<Square>>(rows * columns);
    _rows = rows;
    _columns = columns;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            // The first index of the matrix should be the x position while the second index should be the y position.
            _matrix[flattenPos(j, i)] = Square::alloc(cugl::Vec2(j, i));
        }
    }
    return true;
}

#pragma mark -
#pragma mark Properties
/**
 * Returns if a square exists on the board.
 *
 * @param pos the position of the square.
 * @return whether the square exists.
*/
bool Board::doesSqaureExist(cugl::Vec2 pos) {
    return ((pos.x >= 0) &&
        (pos.x < _columns) &&
        (pos.y >= 0) &&
        (pos.y < _rows));
}

/**
 * Switches position of two squares on the board.
 * NOTE: This does not modify the view of the board. If you want to change the view, you must do so manually.
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2)
{
    shared_ptr<Square> squareOne = getSquare(pos1);
    shared_ptr<Square> squareTwo = getSquare(pos2);
    setSquare(pos2, squareOne);
    setSquare(pos1, squareTwo);
}

/**
 * Switches position of two units while not moving the squares.
 *
 * @param value1  the position of the target square
 * @param value2  the new position of the target square after this switch
 */
void Board::switchUnits(cugl::Vec2 pos1, cugl::Vec2 pos2){
    shared_ptr<Square> squareOne = getSquare(pos1);
    shared_ptr<Square> squareTwo = getSquare(pos2);
    shared_ptr<Unit> unitOne = squareOne->getUnit();
    shared_ptr<Unit> unitTwo = squareTwo->getUnit();
    squareOne->setUnit(unitTwo);
    squareTwo->setUnit(unitOne);
}

/**
     * Switches position of two units and rotate units while not moving the squares.
     * Rotation coresponds to the direction in which the unit is translated.
     * For example a unit who moves to the right will face right.
     *
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
void Board::switchAndRotateUnits(cugl::Vec2 pos1, cugl::Vec2 pos2) {
    Vec2 positionOneUnitDirection = getSquare(pos2)->getPosition() - getSquare(pos1)->getPosition();
    getSquare(pos1)->getUnit()->setDirection(positionOneUnitDirection);
    // The following line is commented out for alpha release (to test only changing direction of originally selected unit). 
//    getSquare(pos2)->getUnit()->setDirection(-1 * positionOneUnitDirection);
    switchUnits(pos1, pos2);
}

/**
* Returns the squares being attacked by a given unit on a square.
*
* @param pos the attacker square's position
* @return a list of squares being attacked.
*/
vector<shared_ptr<Square>> Board::getAttackedSquares(cugl::Vec2 pos) {
    vector<shared_ptr<Square>> result;
    vector<shared_ptr<Square>> ptdResult;

    if (!doesSqaureExist(pos)) {
        return result;
    }
    auto attackingSquare = getSquare(pos);
    for (Vec2 vector : attackingSquare->getUnit()->getBasicAttackRotated()) {
        Vec2 squarePos = vector + attackingSquare->getPosition();
        if (doesSqaureExist(squarePos) && attackingSquare->isInteractable() && (attackingSquare->getUnit()->getColor() != getSquare(squarePos)->getUnit()->getColor() || getSquare(squarePos)->getUnit()->getSubType() == "king")) {
            getAttackedSquares_h(result, ptdResult, getSquare(squarePos));
        }
    }
    return result;
}

void Board::getAttackedSquares_h(vector<shared_ptr<Square>> &listOfAttackedSquares, vector<shared_ptr<Square>> &listOfProtectedSquares, shared_ptr<Square> attackingSquare) {
    if (std::find(listOfAttackedSquares.begin(), listOfAttackedSquares.end(), attackingSquare) != listOfAttackedSquares.end()) {
        return;
    }
    else {
        listOfAttackedSquares.push_back(attackingSquare);
        for (Vec2 vector : attackingSquare->getUnit()->getSpecialAttackRotated()) {
            Vec2 squarePos = vector + attackingSquare->getPosition();
            if (doesSqaureExist(squarePos) && getSquare(squarePos)->isInteractable() && attackingSquare->getUnit()->getColor() == getSquare(squarePos)->getUnit()->getColor() && getSquare(squarePos)->getUnit()->getSubType() != "king"){
                listOfProtectedSquares.push_back(getSquare(squarePos));
            }
            if (doesSqaureExist(squarePos) && getSquare(squarePos)->isInteractable() && (attackingSquare->getUnit()->getColor() != getSquare(squarePos)->getUnit()->getColor() || getSquare(squarePos)->getUnit()->getSubType() == "king")) {
                getAttackedSquares_h(listOfAttackedSquares, listOfProtectedSquares, getSquare(squarePos));
            }
        }
    }
}

vector<shared_ptr<Square>> Board::getInitallyAttackedSquares(cugl::Vec2 pos, bool basic) {
    vector<shared_ptr<Square>> result;
    if (!doesSqaureExist(pos)) {
        return result;
    }
    auto attackingSquare = getSquare(pos);
    vector<Vec2> attackedSquares = basic ? attackingSquare->getUnit()->getBasicAttackRotated() : attackingSquare->getUnit()->getSpecialAttackRotated();
    for (Vec2 vector : attackedSquares) {
        Vec2 squarePos = vector + attackingSquare->getPosition();
        if (doesSqaureExist(squarePos) && attackingSquare->isInteractable() && (attackingSquare->getUnit()->getColor() != getSquare(squarePos)->getUnit()->getColor() || getSquare(squarePos)->getUnit()->getSubType() == "king")) {
            result.push_back(getSquare(squarePos));
        }
    }
    return result;
}

/**
 * Returns the squares being protected in an attack by a given unit on a square.
 *
 * @param pos the attacker square's position
 * @return a list of squares being attacked.
 */
vector<shared_ptr<Square>> Board::getProtectedSquares(cugl::Vec2 pos){
    vector<shared_ptr<Square>> result;
    vector<shared_ptr<Square>> ptdResult;
    if (!doesSqaureExist(pos)) {
        return result;
    }
    auto attackingSquare = getSquare(pos);
    for (Vec2 vector : attackingSquare->getUnit()->getBasicAttackRotated()) {
        Vec2 squarePos = vector + attackingSquare->getPosition();
        if (doesSqaureExist(squarePos) && attackingSquare->isInteractable() && attackingSquare->getUnit()->getColor() == getSquare(squarePos)->getUnit()->getColor() && getSquare(squarePos)->getUnit()->getSubType() != "king"){
            ptdResult.push_back(getSquare(squarePos));
        }
        if (doesSqaureExist(squarePos) && attackingSquare->isInteractable() && (attackingSquare->getUnit()->getColor() != getSquare(squarePos)->getUnit()->getColor() || getSquare(squarePos)->getUnit()->getSubType() == "king")) {
            getAttackedSquares_h(result,ptdResult, getSquare(squarePos));
        }
    }
    return ptdResult;
}

vector<shared_ptr<Square>> Board::getInitiallyProtectedSquares(cugl::Vec2 pos, bool basic) {
    CULog("call get protected");
    vector<shared_ptr<Square>> result;
    if (!doesSqaureExist(pos)) {
        return result;
    }
    auto attackingSquare = getSquare(pos);
    vector<Vec2> attackedSquares = basic ? attackingSquare->getUnit()->getBasicAttackRotated() : attackingSquare->getUnit()->getSpecialAttackRotated();
    for (Vec2 vector : attackedSquares) {
        Vec2 squarePos = vector + attackingSquare->getPosition();
        if (doesSqaureExist(squarePos) && attackingSquare->isInteractable() && attackingSquare->getUnit()->getColor() == getSquare(squarePos)->getUnit()->getColor() && getSquare(squarePos)->getUnit()->getSubType() != "king") {
            result.push_back(getSquare(squarePos));
            CULog("pushed");
        }
    }
    return result;
}
