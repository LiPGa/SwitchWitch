//
//  SWSquare.hpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWSquare_hpp
#define SWSquare_hpp
#include <cugl/cugl.h>
#include "SWUnit.hpp"

/**
 * Model class representing a square.
 */
class Square
{
private:
    /** Position of this square*/
    cugl::Vec2 _pos;

    /** Unit on this square*/
    shared_ptr<Unit> _unit;

    /** The Polygon-Node that represents this square */
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
    Square(){}

    /**
     * Disposes the square. releasing all resources
     */
    ~Square() {}

#pragma mark -
#pragma mark Initializers
    /**
     * Creats a square with the given position
     * 
     * @param position of the square
     * @return if the initialization was successful
     */
    bool initWithPos(const cugl::Vec2& pos);

    /**
     * Creats a square with the given position and unit
     * 
     * @param position of the square
     * @param unit on the square
     * @return if the initialization was successful
     */
    bool initWithPosAndUnit(const cugl::Vec2& pos, shared_ptr<Unit> unit);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated square.
     *
     * @param position of the Square.
     * @return a newly allocated Square.
     */
    static std::shared_ptr<Square>alloc(cugl::Vec2 position) {
        std::shared_ptr<Square> result = std::make_shared<Square>();
        return (result->initWithPos(position) ? result : nullptr);
    }

    /**
     * Returns a newly allocated square with a unit.
     *
     * @param position of the Square.
     * @param a unit.
     * @return a newly allocated Square.
     */
    static std::shared_ptr<Square>alloc(cugl::Vec2 position, shared_ptr<Unit> unit) {
        std::shared_ptr<Square> result = std::make_shared<Square>();
        return (result->initWithPosAndUnit(position, unit) ? result : nullptr);
    }

#pragma mark -
#pragma mark Identifiers
    /**
     * Returns the position of the square relative to the board.
     * 
     * This is location of the center of the square.
     *
     * @return position of the square (Vec2)
     */
    const cugl::Vec2 &getPosition() const { return _pos; }

    /**
     * Sets the position of this square.
     *
     * This is location of the center of the square.
     *
     * @param value the position of this square.
     */
    void setPosition(cugl::Vec2 position) { _pos = position; }

    /**
     * Returns the unit on this square.
     *
     * @return the unit on this square
     */
    shared_ptr<Unit> getUnit() { return _unit; }

    /**
     * Sets the unit on this square.
     *
     * @param value the unit on this ship
     */
    void setUnit(shared_ptr<Unit> unit) { _unit = unit; }
    
    /**
     * Returns the Polygon-Node that represents the square's view.
     *
     * @return square's Polygon-Node
     */
    shared_ptr<cugl::scene2::PolygonNode> getViewNode() { return _viewNode; }

    /**
     * Sets the Polygon-Node that represents the square's view.
     *
     * @param a polygon-node for the square.
     */
    void setViewNode(shared_ptr<cugl::scene2::PolygonNode> viewNode) { _viewNode = viewNode; }

#pragma mark -
#pragma mark Operator
    /**
     * Override the == operator
     */
    bool operator==(shared_ptr<Square> square) const {
        if (this->_pos.x == square->getPosition().x && this->_pos.y == square->getPosition().y)
            return true;
        else return false;
    }
};

#endif /* SWSquare_hpp */
