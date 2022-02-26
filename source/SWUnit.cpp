//
//  SWUnit.cpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWUnit.hpp"
using namespace cugl;

//const cugl::Vec2 defaultDerection = Vec2::UNIT_X;

/**
 * Creates a unit with the given color and attack pattern.
 *
 * @param color the unit color
 * @param basicAttack the basic attack pattern of the unit
 * @param specialAttack the special attack pattern of the unit
 * @param direction the direction the unit is facing
 */
Unit::Unit(const Colors color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction)
{
    defaultDirection = Vec2::UNIT_X; 
    this->color = color;
    this->basicAttack = basicAttack;
    this->direction = direction;
    this->specialAttack = specialAttack;
}
