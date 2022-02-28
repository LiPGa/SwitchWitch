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
    /** The default direction of a unit.
     * This default direction repesents the direction the unit is assumed to be facing,
     * when inputing information reguarding the direction of attacks.
     * In other words, if the unit is facing in the default direction, all vectors
     * that represent the direction the unit should attack are not rotated.
     * If the unit is facing a direction different from the default direction, 
     * all attack vectors are rotated so that an attack faces the direction of the unit.
     * 
     * Currently, the default direction is Vec2(1,0). All units should have the same default direction.
    */
    cugl::Vec2 defaultDirection;


public:
    /** Available colors for a unit. Each unit will have one of the three colors.*/
    enum Colors
    {
        red,
        green,
        blue
    };

private:
    /**Both the basic attack and special attack of a unit is stored as a list of vectors. 
     * Each vector repesents a distance and direction of an attack. The direction is based on the
     * default direction, meaning that all attack data should be inputed assuming that the unit is facing
     * in the default direction.
     * 
     * Attacks are assumed to kill any unit instantly.
     */

    /** The basic attacks of this unit*/
    vector<cugl::Vec2> basicAttack;
    
    /** The special attacks of this unit*/
    vector<cugl::Vec2> specialAttack;

    /** The direction this unit is currently facing.*/
    cugl::Vec2 direction;

    /** The color of this unit.*/
    Colors color;
    

#pragma mark Constructors
public:
    Unit() {}
    /**
     * Creates a unit given a color, direction, basic attack pattern, and special attack pattern.
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


    /**
     * Returns a list of vec2 representing all attacks
     * in a basic attack. The direction of attacks is based on the units default direction.
     * 
     * @return unit's basic attack pattern
     */
    vector<cugl::Vec2> getBasicAttack()
    {
        return basicAttack;
    }
    /**
     * * Returns a list of vec2 representing all attacks
     * in a special attack. The direction of attacks is based on the units default direction.
     * 
     * @return unit's special attack pattern
     */
    vector<cugl::Vec2> getSpecialAttack()
    {
        return specialAttack;
    }
    /**
     * @return unit's color
     */
    Color4 getColor()
    {
        switch(color) {
            case red: return Color4::RED;
            case green: return Color4::GREEN;
            case blue: return Color4::BLUE;
            default: return Color4::RED;
        }
    }
    /**
     * Returns the direction which the unit is currently facing.
     * 
     * @return unit's direction
     */
    cugl::Vec2 getDirection()
    {
        return direction;
    }
    /**
     * Sets the unit's basic attack. The basic attack is represented as a list of vec2 representing
     * the direction and distance of an attack from its square.
     * The attack pattern should assume that the unit is facing in the default direction.
     *
     * @param attack the basic attack pattern of this unit.
     */
    void setBasicAttack(vector<cugl::Vec2> attack)
    {
        basicAttack = attack;
    }
    /**
     * Sets the unit's special attack. The special attack is represented as a list of vec2 representing
     * the direction and distance of an attack from its square.
     * The attack pattern should assume that the unit is facing in the default direction.
     *
     * @param attack
     */
    void setSpecialAttack(vector<cugl::Vec2> attack)
    {
        specialAttack = attack;
    }
    /**
     *Sets the unit's color
     *
     * @param c the color of the unit.
     */
    void setColor(Colors c)
    {
        color = c;
    }
    /**
     * Sets the unit's current direction.
     *
     * @param d the direction the unit is facing. 
     * Must be a unit vector that represents one of the 4 cardinal directions.
     */
    void setDirection(cugl::Vec2 d)
    {
        direction = d;
    }
};

#endif /* SWUnit_hpp */
