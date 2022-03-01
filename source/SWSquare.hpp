//
//  SWSquare.hpp
//  ShipLab
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
    Unit _unit;

public:
#pragma mark Constructors
    Square(){}
    /**
     * Creats a square with the given position and unit
     */
    Square(const cugl::Vec2 &pos, Unit &unit);
    
    /**
     * Creats a square with the given position
     */
    Square(const cugl::Vec2 &pos);

    /**
     * Disposes the square. releasing all resources
     */
    ~Square() {}

#pragma mark Properties
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
    void setPosition(cugl::Vec2 value) { _pos = value; }

    /**
     * Returns the unit on this square.
     *
     * @return the unit on this square
     */
    Unit getUnit() { return _unit; }

    /**
     * Sets the unit on this square.
     *
     * @param value the unit on this ship
     */
    void setUnit(Unit value) { _unit = value; }
    
    /**
     * Override the == operator
     */
    bool operator==(const Square& s) const {
        if (this->_pos.x == s.getPosition().x && this->_pos.y == s.getPosition().y)
            return true;
        else return false;
    }
};

#endif /* SWSquare_hpp */
