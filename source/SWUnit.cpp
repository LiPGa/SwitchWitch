//
//  SWUnit.cpp
//  ShipLab
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWUnit.hpp"
using namespace cugl;

/**
 * Creates a unit with the given color and attack pattern.
 *
 * @param color The unit color
 * @param attack The attack pattern of the unit
 */
Unit::Unit(const Colors color, vector<cugl::Vec2> attack, cugl::Vec2 direction) {
    _color = color;
    _attack = attack;
    _direction = direction;
}

//basicAttack.push_back(Vec2 (0, 1));
//doubleAttack.push_back(Vec2 (0, 1));
//doubleAttack.push_back(Vec2 (0, 2));
//tripleAttack.push_back(Vec2 (-1, 1));
//tripleAttack.push_back(Vec2 (0, 1));
//tripleAttack.push_back(Vec2 (1, 1));
//cornerAttack.push_back(Vec2 (-1, 1));
//cornerAttack.push_back(Vec2 (-1, -1));
//cornerAttack.push_back(Vec2 (1, 1));
//cornerAttack.push_back(Vec2 (1, -1));
