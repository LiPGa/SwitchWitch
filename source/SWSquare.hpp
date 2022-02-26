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
     * Disposes the square. releasing all resources
     */
    ~Square() {}

#pragma mark Properties
    /**
     * Returns the posiSquarehis square.
     *
     * This is locationSquareenter pixel of the square Squaren.
     *
     * @retuSquaresition of this square
     */
    const cugl::Vec2 &getPosition() const { return _pos; }

    /**
     * Sets the position of this square.
     *
     * This is location of the center pixel of the square on the screen.
     *
     * @param value the position of this ship
     */
    void setPosition(cugl::Vec2 value) { _pos = value; }

    /**
     * Returns the unit on this square.
     *
     * @return the unit on this square
     */
    const Unit getUnit() const { return _unit; }

    /**
     * Sets the unit on this square.
     *
     * @param value the unit on this ship
     */
    void setUnit(Unit value) { _unit = value; }
};

#endif /* SWSquare_hpp */
