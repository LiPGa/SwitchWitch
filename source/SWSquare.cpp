//
//  SWSquare.cpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWSquare.hpp"

using namespace cugl;

/**
 * Creates a square wiht the given position and unit.
 *
 * @param pos   The square position
 * @param unit  The unit on the square
 */
Square::Square(const cugl::Vec2 &pos, Unit &unit)
{
    _pos = pos;
    _unit = unit;
}


