#ifndef SWBoard_hpp
#define SWBoard_hpp

#include "SWSquare.hpp"
#include <cugl/cugl.h>

class Board
{
private:
    /** The number of rows on the board */
    int _rows;

    /** The number of columns on the board */
    int _columns;

    /** Square matrix on this board */
    vector<shared_ptr<Square>> _matrix;

    /** The Polygon-Node that represents this board */
    shared_ptr<cugl::scene2::PolygonNode> _viewNode;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized Unit.
     *
     * You must initialize this Unit before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Unit on the
     * heap, use one of the static constructors instead.
     */
    Board() {};

    /**
     * Disposes the board, releasing all resources
     */
    ~Board() {}

    /**
     * Initializes a board with the given size
     *
     * @param rows The number of rows on the board.
     * @param columns The number of columns on the board.
     */
    bool init(int rows, int columns);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated board.
     * NOTE: It will also allocate squares for you so that all points on the board will have a square. 
     * Units in squares are not allocated.
     * 
     * @param rows the number of rows on the board.
     * @param columns the number of columns on the board.
     * @return a newly allocated Board.
     */
    static shared_ptr<Board> alloc(int rows, int columns) {
        std::shared_ptr<Board> result = std::make_shared<Board>();
        return (result->init(rows, columns) ? result : nullptr);
    }

#pragma mark -
#pragma mark Identifiers
    /**
     * Returns if a square exists on the board.
     *
     * @param pos the position of the square.
     * @return whether the square exists.
     */
    bool doesSqaureExist(cugl::Vec2 pos);

    /**
     * Returns the square at a position
     *
     * @param _pos the position of a square
     * @return the square at that position
     */
    shared_ptr<Square> getSquare(cugl::Vec2 position) {
        return _matrix[flattenPos(position.x, position.y)];
    }

    /**
     * Returns all squares from the board
     *
     * @return a vector of all squares.
     */
    vector<shared_ptr<Square>> getAllSquares() { return _matrix; }

    /**
     * Sets a square in a position on the board. 
     * Note: the square will automatically be set to the position specified in the parameters. 
     * 
     * @param position The position that the square should replace.
     * @param square The square that will be set at the position of the board.
     */
    void setSquare(cugl::Vec2 position, shared_ptr<Square> square) { 
        square->setPosition(position);
        _matrix[flattenPos(position.x, position.y)] = square;
    }

    /**
     * Sets a square in a position on the board.
     *
     * @param square The square that will be set at the position of the board.
     */
    void setSquare(shared_ptr<Square> square) { setSquare(square->getPosition(), square); }
    
    /**
     * Returns the Polygon-Node that represents the board's view.
     *
     * @return board's Polygon-Node
     */
    shared_ptr<cugl::scene2::PolygonNode> getViewNode() { return _viewNode; }

    /**
     * Sets the Polygon-Node that represents the board's view.
     *
     * @param a polygon-node for the board.
     */
    void setViewNode(shared_ptr<cugl::scene2::PolygonNode> viewNode) { _viewNode = viewNode; }

    /**
     * Switches position of two squares on the board.
     *
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
    void switchSquares(cugl::Vec2 pos1, cugl::Vec2 pos2);
    
    /**
     * Switches position of two units while not moving the squares.
     * 
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
    void switchUnits(cugl::Vec2 pos1, cugl::Vec2 pos2);

    /**
     * Switches position of two units and rotate units while not moving the squares.
     * Rotation coresponds to the direction in which the unit is translated.
     * For example a unit who moves to the right will face right.
     * 
     * @param value1  the position of the target square
     * @param value2  the new position of the target square after this switch
     */
    void switchAndRotateUnits(cugl::Vec2 pos1, cugl::Vec2 pos2);

    /**
     * Returns the squares being attacked by a given unit on a square.
     *
     * @param pos the attacker square's position
     * @return a list of squares being attacked.
     */
    vector<shared_ptr<Square>> getAttackedSquares(cugl::Vec2 pos);

private: 
    /**
     * Flattens two coordinate number as one number. Used for organizing the _matrix.
     */
    int flattenPos(int x, int y) { return x * _rows + y; }
    /**
     * Helper function for finding attacked squares.
     */
    void getAttackedSquares_h(vector<shared_ptr<Square>> &listOfAttackedSquares, shared_ptr<Square> attackingSquare);
#pragma mark -
};
#endif /* SWBoard_hpp */
