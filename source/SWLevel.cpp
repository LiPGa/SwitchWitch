//
//  SWLevel.cpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWLevel.h"
using namespace cugl;

#pragma mark Constructors
/**
 * Initializes a level with the given board size
 *
 * @param rows The number of rows on the board.
 * @param columns The number of columns on the board.
 */
bool Level::init(int rows, int columns) {
    _rows = rows;
    _columns = columns;
    levelID = 0;
    maxTurns = 0;
    oneStarThreshold = 0;
    twoStarThreshold = 0;
    threeStarThreshold = 0;
    kingThreshold = 0;
    return true;
}

/**
 * Initializes a level given JSON values
 *
 * @param JSON values
 */
bool Level::init(int rows, int columns, shared_ptr<JsonValue> levelJSON) {
    levelID = levelJSON->getInt("id");
    maxTurns = levelJSON->getInt("total-swap-allowed");
    // thresholds for the star system
    oneStarThreshold = levelJSON->getInt("one-star-condition");
    twoStarThreshold = levelJSON->getInt("two-star-condition");
    threeStarThreshold = levelJSON->getInt("three-star-condition");
    auto layersInBoardJson = levelJSON->get("board-members")->children();
    for (auto layer : layersInBoardJson)
    {
        auto board = Board::alloc(rows, columns);
        auto units = layer->children();
        for (auto i = 0; i < units.size(); i++) {
            auto sq = board->getAllSquares()[i];
            auto unitDirection = Vec2(units[i]->get("direction")->asFloatArray().at(0), units[i]->get("direction")->asFloatArray().at(1));
            auto unitColor = units[i]->getString("color");
            auto unitSubType = units[i]->getString("sub-type");
            sq->setInteractable(unitSubType != "empty");
            sq->setUnit(Unit::alloc(unitSubType, Unit::stringToColor(unitColor), unitDirection));
        }
        addBoard(board);
    }
    return true;
}

/**
 * Converts the level to a JsonValue
 */
shared_ptr<JsonValue> Level::convertToJSON() {
    shared_ptr<cugl::JsonValue> boardJSON = cugl::JsonValue::allocObject();
    boardJSON->appendChild("id", cugl::JsonValue::alloc((long int)levelID));
    boardJSON->appendChild("total-swap-allowed", cugl::JsonValue::alloc((long int)maxTurns));
    boardJSON->appendChild("one-star-condition", cugl::JsonValue::alloc((long int)oneStarThreshold));
    boardJSON->appendChild("two-star-condition", cugl::JsonValue::alloc((long int)twoStarThreshold));
    boardJSON->appendChild("three-star-condition", cugl::JsonValue::alloc((long int)threeStarThreshold));
    //boardJSON->appendChild("king-threshold", cugl::JsonValue::alloc((long int)kingThreshold));
    shared_ptr<cugl::JsonValue> boardArray = cugl::JsonValue::allocArray();
    for (shared_ptr<Board> board : _boards) {
        shared_ptr<cugl::JsonValue> squareOccupantArray = cugl::JsonValue::allocArray();
        for (shared_ptr<Square> square : board->getAllSquares()) {
            auto unit = square->getUnit();
            auto unitJSON = cugl::JsonValue::allocObject();
            unitJSON->appendChild("type", cugl::JsonValue::alloc("unit"));
            unitJSON->appendChild("sub-type", cugl::JsonValue::alloc(unit->getSubType()));
            unitJSON->appendChild("color", cugl::JsonValue::alloc(Unit::colorToString(unit->getColor())));
            auto unitDirectionJSON = cugl::JsonValue::allocArray();
            unitDirectionJSON->appendChild(cugl::JsonValue::alloc(unit->getDirection().x));
            unitDirectionJSON->appendChild(cugl::JsonValue::alloc(unit->getDirection().y));
            unitJSON->appendChild("direction", unitDirectionJSON);
            squareOccupantArray->appendChild(unitJSON);
        }
        boardArray->appendChild(squareOccupantArray);
    }
    boardJSON->appendChild("board-members", boardArray);
    return boardJSON;
}

#pragma mark -
#pragma mark Properties
/**
 * Gets the square that should replace the current square.
 *
 * @param the position of the square.
 * @return the replacement square
 */
shared_ptr<Square> Level::getReplacementSquare(cugl::Vec2 squarePosition, int depth) {
    return _boards[depth]->getSquare(squarePosition);
}

/**
 * Returns the number of stars achieved given the score
 *
 * @param the score
 * @return the number of stars
 */
int Level::getNumberOfStars(int score) {
    if (score >= threeStarThreshold) {
        return 3;
    }
    else if (score >= twoStarThreshold) {
        return 2;
    }
    else if (score >= oneStarThreshold) {
        return 1;
    }
    else {
        return 0;
    }
}

#pragma mark -
