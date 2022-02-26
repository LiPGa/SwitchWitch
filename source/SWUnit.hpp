//
//  SWUnit.hpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWUnit_hpp
#define SWUnit_hpp

#include <cugl/cugl.h>
using namespace cugl;

/**
 * Model class representing an unit.
 */
class Unit
{
public:
    static const cugl::Vec2 defaultDirection;

public:
    /** Available colors for a unit*/
    enum Colors
    {
        red,
        green,
        blue
    };

private:
    /** Available attack pattern of a unit, suppose the position of the attacker is (0, 0) and they are facing south*/
    vector<cugl::Vec2> basicAttack;

    vector<cugl::Vec2> specialAttack;

    cugl::Vec2 direction;

    Colors color;

#pragma mark Constructors
public:
    /**
     * Creates a unit with the given color and attck pattern
     *
     * @param color the unit color
     * @param basicAttack the basic attack pattern of the unit
     * @param specialAttack the special attack pattern of the unit
     * @param direction the direction the unit is facing
     */
    Unit(const Colors color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction);

    /**
     * Disposes the unit, releasing all resources
     */
    ~Unit() {}

    Unit() {}
    /**
     * @return unit's basic attack
     */
    vector<cugl::Vec2> getBasicAttack()
    {
        return basicAttack;
    }
    /**
     * @return unit's special attack
     */
    vector<cugl::Vec2> getSpecialAttack()
    {
        return specialAttack;
    }
    /**
     * @return unit's color
     */
    Colors getColor()
    {
        return color;
    }
    /**
     * @return unit's direction
     */
    cugl::Vec2 getDirection()
    {
        return direction;
    }
    /**
     *Sets unit's basic attack
     *
     * @param attack
     */
    void setBasicAttack(vector<cugl::Vec2> attack)
    {
        basicAttack = attack;
    }
    /**
     *Sets unit's special attack
     *
     * @param attack
     */
    void setSpecialAttack(vector<cugl::Vec2> attack)
    {
        specialAttack = attack;
    }
    /**
     *Sets unit's color
     *
     * @param c for color
     */
    void setColor(Colors c)
    {
        color = c;
    }
    /**
     *Sets unit's direction d
     *
     * @param attack
     */
    void setDirection(cugl::Vec2 d)
    {
        direction = d;
    }
};

#endif /* SWUnit_hpp */
