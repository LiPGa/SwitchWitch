//
//  SWLevel.cpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright � 2022 Game Design Initiative at Cornell. All rights reserved.
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
bool Level::init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, int columns, int rows) {
    _rows = rows;
    _columns = columns;
    levelID = 0;
    maxTurns = 0;
    oneStarThreshold = 0;
    twoStarThreshold = 0;
    threeStarThreshold = 0;
    squareSize = 162;
    backgroundName = "forest";
    return true;
}

/**
 * Initializes a level given JSON values
 *
 * @param JSON values
 */
bool Level::init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, shared_ptr<JsonValue> levelJSON) {
    levelID = levelJSON->getInt("id");
    maxTurns = levelJSON->getInt("total-swap-allowed");
    // Rows and Columns
    _rows = levelJSON->getInt("rows");
    _columns = levelJSON->getInt("columns");
    // thresholds for the star system
    oneStarThreshold = levelJSON->getInt("one-star-condition");
    twoStarThreshold = levelJSON->getInt("two-star-condition");
    threeStarThreshold = levelJSON->getInt("three-star-condition");
    numOfKings = levelJSON->getInt("num-of-kings");
    backgroundName = levelJSON->getString("background");
    unitTypes = levelJSON->get("unit-types")->asStringArray();
    squareSize = levelJSON->getInt("square-size");
    auto layersInBoardJson = levelJSON->get("board-members")->children();
    
    for (auto layer : layersInBoardJson)
    {
        auto board = Board::alloc(_columns, _rows);
        auto units = layer->children();
        for (auto i = 0; i < units.size(); i++) {
            auto sq = board->getAllSquares()[i];
            auto unitDirection = Vec2(units[i]->get("direction")->asFloatArray().at(0), units[i]->get("direction")->asFloatArray().at(1));
            auto unitColorStr = units[i]->getString("color");
            auto unitSubType = units[i]->getString("sub-type");
            auto unitsNeededToKill = units[i]->getInt("unitsNeededToKill");
            sq->setInteractable(unitSubType != "empty");
            auto unitColor = unitSubType == "king" ? Unit::Color::NONE : Unit::stringToColor(unitColorStr);
            sq->setUnit(Unit::alloc(textures, unitSubType, unitColor, unitDirection, unitSubType != "king", unitSubType != "basic" && unitSubType != "random" && unitSubType != "king", unitsNeededToKill));
        }
        auto test = addBoard(board);
    }
    return true;
}

/**
 * Converts the level to a JsonValue
 */
shared_ptr<JsonValue> Level::convertToJSON() {
    shared_ptr<cugl::JsonValue> boardJSON = cugl::JsonValue::allocObject();
    boardJSON->appendChild("id", cugl::JsonValue::alloc((long int)levelID));
    boardJSON->appendChild("rows", cugl::JsonValue::alloc((long int)_rows));
    boardJSON->appendChild("columns", cugl::JsonValue::alloc((long int)_columns));
    boardJSON->appendChild("total-swap-allowed", cugl::JsonValue::alloc((long int)maxTurns));
    boardJSON->appendChild("one-star-condition", cugl::JsonValue::alloc((long int)oneStarThreshold));
    boardJSON->appendChild("two-star-condition", cugl::JsonValue::alloc((long int)twoStarThreshold));
    boardJSON->appendChild("three-star-condition", cugl::JsonValue::alloc((long int)threeStarThreshold));
    boardJSON->appendChild("background", cugl::JsonValue::alloc(backgroundName));
    boardJSON->appendChild("square-size", cugl::JsonValue::alloc((long int)squareSize));
    auto unitTypeArray = cugl::JsonValue::allocArray();
    unitTypeArray->appendChild(cugl::JsonValue::alloc("empty"));
    unitTypeArray->appendChild(cugl::JsonValue::alloc("empty"));
    unitTypeArray->appendChild(cugl::JsonValue::alloc("empty"));
    boardJSON->appendChild("unit-types", unitTypeArray);
    numOfKings = 0;
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
            unitJSON->appendChild("unitsNeededToKill", cugl::JsonValue::alloc((long int)unit->getUnitsNeededToKill()));
            squareOccupantArray->appendChild(unitJSON);
            if (unit->getSubType() == "king") numOfKings++;
        }
        boardArray->appendChild(squareOccupantArray);
    }
    boardJSON->appendChild("board-members", boardArray);
    boardJSON->appendChild("num-of-kings", cugl::JsonValue::alloc((long int)numOfKings));
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
