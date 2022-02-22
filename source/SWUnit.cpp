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
Unit::Unit(const Colors color, Attacks attack, Directions direction) {
    _color = color;
    _attack = attack;
    _direction = direction;
}
