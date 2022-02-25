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
class Unit {
public:
    /** Available colors for a unit*/
    enum Colors {red, green, blue};
    
    /** Available attack pattern of a unit, suppose the position of the attacker is (0, 0) and they are facing south*/
    static vector<cugl::Vec2> basicAttack;
    basicAttack = {Vec2(0, 1)};
    
    vector<cugl::Vec2> doubleAttack = {Vec2(0, 1), Vec2(0, 2)};
    vector<cugl::Vec2> tripleAttack = {Vec2(-1, 1), Vec2(0, 1), Vec2(1, 1)};
    vector<cugl::Vec2> cornerAttack = {Vec2(-1, 1), Vec2(-1, -1), Vec2(1, 1), Vec2(1, -1)};
    
    /** Available directions of a spefical unit*/
    cugl::Vec2 up = Vec2(0,-1);
    cugl::Vec2 down = Vec2(0,1);
    cugl::Vec2 left = Vec2(-1,0);
    cugl::Vec2 right = Vec2(0,1);
    
    //    basicAttack.push_back(Vec2 (0, 1));
    //    doubleAttack.push_back(Vec2 (0, 1));
    //    doubleAttack.push_back(Vec2 (0, 2));
    //    tripleAttack.push_back(Vec2 (-1, 1));
    //    tripleAttack.push_back(Vec2 (0, 1));
    //    tripleAttack.push_back(Vec2 (1, 1));
    //    cornerAttack.push_back(Vec2 (-1, 1));
    //    cornerAttack.push_back(Vec2 (-1, -1));
    //    cornerAttack.push_back(Vec2 (1, 1));
    //    cornerAttack.push_back(Vec2 (1, -1));
    
private:
    /** Color of the unit*/
    Colors _color;
    
    vector<cugl::Vec2> _attack;
    
    cugl::Vec2 _direction;


#pragma mark Constructors
public:
    /**
     * Creates a unit with the given color and attck pattern
     *
     * @param color the unit color
     * @param attack the attack pattern of the unit
     */
    Unit(const Colors color, vector<cugl::Vec2> attack, Vec2 direction);
    /**
     * Creates a defult unit
     */
    Unit() {
        _color = red;
        _attack = basicAttack;
        _direction = down;
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
    const vector<cugl::Vec2> getAttack() const {return _attack;}
    
    /**
     * Sets the attack of this unit
     * @param value the attack pattern of this unit
     */
    void setAttack(vector<cugl::Vec2> value) {_attack = value;}
    
    /**
     * Returns the direction of this special unit
     *
     * @return the direction of this unit
     */
    const cugl::Vec2 getDirection() {return _direction;}
    
    /**
     * Sets the attack of this unit
     * @param value the attack pattern of this unit
     */
    void setDirection(cugl::Vec2 value) {_direction = value;}
    
};

#endif /* SWUnit_hpp */
