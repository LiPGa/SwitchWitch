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

/**
 * Model class representing an unit.
 */
class Unit {
private:
    /** Color of the unit*/
    Colors _color;
    
    /** Attack pattern of a unit*/
    Attacks _attack;
    
    /** Facing direction of a special unit*/
    Directions _direction;
    
public:
    /** Available colors for a unit*/
    enum Colors {red, green, blue};
    
    /** Available attack pattern of a unit*/
    enum Attacks {basicAttack, doubleAttack, tripleAttack, cornerAttack};
    
    /** Available directions of a spefical unit*/
    enum Directions {na, up, down, left, right};
    
#pragma mark Constructors
    /**
     * Creates a unit with the given color and attck pattern
     *
     * @param color the unit color
     * @param attack the attack pattern of the unit
     */
    Unit(const Colors color, Attacks attack, Directions direction);
    /**
     * Creates a defult unit
     */
    Unit() {
        _color = red;
        _attack = basicAttack;
        _direction = na;
    }
    
    /**
     * Disposes the unit, releasing all resources
     */
    ~Unit() {}

#pragma mark Properties
    /**
     * Returns the color of this unit
     *
     * @return the color of this unit
     */
    const Colors getColor() const {return _color;}
    
    /**
     * Sets the color of this unit
     * @param value the color of this unit
     */
    void setColor(Colors value) {_color = value;}
    
    /**
     * Returns the attack pattern of this unit
     *
     * @return the attack pattern of this unit
     */
    const Attacks getAttack() const {return _attack;}
    
    /**
     * Sets the attack of this unit
     * @param value the attack pattern of this unit
     */
    void setAttack(Attacks value) {_attack = value;}
    
    /**
     * Returns the direction of this special unit
     *
     * @return the direction of this unit
     */
    const Directions getDirection() const {return _direction;}
    
    /**
     * Sets the attack of this unit
     * @param value the attack pattern of this unit
     */
    void setDirection(Directions value) {_direction = value;}
    
};

#endif /* SWUnit_hpp */
