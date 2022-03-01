//
//  SWUnit.cpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWUnit.hpp"
using namespace cugl;

/** The default direction of a unit.
 * This default direction repesents the direction the unit is assumed to be facing,
 * when inputing information reguarding the direction of attacks.
 * In other words, if the unit is facing in the default direction, all vectors
 * that represent the direction the unit should attack are not rotated.
 * If the unit is facing a direction different from the default direction, 
 * all attack vectors are rotated so that an attack faces the direction of the unit.
 * 
 * Currently, the default directio is Vec2(1,0).
*/
//const cugl::Vec2 defaultDerection = Vec2::UNIT_X;

Unit::Unit(){
    defaultDirection = Vec2::UNIT_X;
    this->_color = RED;
    this->_direction = Vec2::UNIT_X;
}

/**
 * Creates a unit given a color, direction, basic attack pattern, and special attack pattern.
 *
 * @param color the unit color
 * @param basicAttack the basic attack pattern of the unit
 * @param specialAttack the special attack pattern of the unit
 * @param direction the direction the unit is facing
 */
Unit::Unit(const Color color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction)
{
    defaultDirection = Vec2::UNIT_X; 
    this->_color = color;
    this->_basicAttack = basicAttack;
    this->_direction = direction;
    this->_specialAttack = specialAttack;
}
