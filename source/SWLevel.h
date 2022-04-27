#pragma once
//
//  SWLevel.hpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright � 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWLevel_h
#define SWLevel_h

#include "SWBoard.hpp"
#include <cugl/cugl.h>

class Level
{
private:
    /** Boards store the initial board and all replacement boards */
    vector<shared_ptr<Board>> _boards;

    int _rows;
    int _columns;

public:
    /** The ID of the level */
    int levelID;
    /** The maximum number of turns  */
    int maxTurns;
    /** one-star threshold */
    int oneStarThreshold;
    /** two-star threshold*/
    int twoStarThreshold;
    /** three-star threshold*/
    int threeStarThreshold;
    /** number of kings*/
    int numOfKings;

#pragma mark Constructors
    /**
     * Creates an uninitialized Unit.
     *
     * You must initialize this Unit before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Unit on the
     * heap, use one of the static constructors instead.
     */
    Level() {};

    /**
     * Disposes the board, releasing all resources
     */
    ~Level() {}

    /**
     * Initializes a level with the given board size
     *
     * @param rows The number of rows on the board.
     * @param columns The number of columns on the board.
     */
    bool init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, int columns, int rows);

    /**
     * Initializes a level given JSON values
     *
     * @param JSON values
     */
    bool init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, shared_ptr<JsonValue> levelJSON);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated level with a blank initial board.
     * NOTE: It will also allocate squares for you so that all points on the board will have a square.
     * Units in squares are not allocated.
     *
     * @param rows the number of rows on the board.
     * @param columns the number of columns on the board.
     * @return a newly allocated Board.
     */
    static shared_ptr<Level> alloc(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, int columns, int rows) {
        std::shared_ptr<Level> result = std::make_shared<Level>();
        return (result->init(textures, columns, rows) ? result : nullptr);
    }

    /**
     * Returns a newly allocated level with all information. 
     * NOTE: This will not allocate any view nodes. This model only stores level data. It should not store any view information.
     *
     * @param rows the number of rows on the board.
     * @param columns the number of columns on the board.
     * @return a newly allocated Board.
     */
    static shared_ptr<Level> alloc(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, shared_ptr<JsonValue> levelJSON) {
        std::shared_ptr<Level> result = std::make_shared<Level>();
        return (result->init(textures, levelJSON) ? result : nullptr);
    }

#pragma mark -
#pragma mark Identifiers
    /**
     * Get number of rows.
     */
    int getNumberOfRows() { return _rows; }

    /**
     * Get number of columns.
     */
    int getNumberOfColumns() { return _columns; }

    /**
     * Adds a blank board to the level, increaseing the depth of the level.
     */
    void addBlankBoard() { _boards.push_back(Board::alloc(_columns, _rows)); }

    /**
     * Adds a board to the level, increaseing the depth of the level. Returns whether the addition was successful.
     */
    bool addBoard(shared_ptr<Board> board) { 
        bool valid = board->getNumberOfRows() == _rows && board->getNumberOfColumns() == _columns;
        auto rows = board->getNumberOfRows();
        auto columns = board->getNumberOfColumns();
        if (valid) {
            _boards.push_back(board);
        }
        return valid;
    }

    /**
     * Removes a board at a certain depth
     */
    void removeBoard(int depth) { _boards.erase(_boards.begin() + depth); }

    /** 
     * Returns whether a board exists at a certain depth.
     * 
     * @param the depth of the board
     * @return whether a board exists at a certain depth.
     */
    bool doesBoardExist(int depth) { return depth < _boards.size(); }

    /**
     * Gets a board at a certain depth. 
     * 
     * @param the depth of the board
     * @return the board at that depth
     */
    shared_ptr<Board> getBoard(int depth) { return _boards[depth]; }

    /** 
     * Gets the square that should replace the current square.
     * 
     * @param the position of the square.
     * @return the replacement square
     */
    shared_ptr<Square> getReplacementSquare(cugl::Vec2 squarePosition, int depth);

    /**
     * Returns the number of stars achieved given the score
     * 
     * @param the score
     * @return the number of stars
     */
    int getNumberOfStars(int score);

    /**
     * Converts the level to a JsonValue
     */
    shared_ptr<JsonValue> convertToJSON();
#pragma mark -
};
#endif /* SWBoard_hpp */
