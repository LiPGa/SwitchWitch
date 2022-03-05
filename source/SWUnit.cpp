#include "SWUnit.hpp"
using namespace cugl;


/**
 * Initializes a unit given a color, direction, basic attack pattern, and special attack pattern.
 *
 * @param color the unit color
 * @param basicAttack the basic attack pattern of the unit
 * @param specialAttack the special attack pattern of the unit
 * @param direction the direction the unit is facing
 * @return true if initialization was successful.
 */
bool Unit::init(const Color color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction)
{
    this->_color = color;
    this->_basicAttack = basicAttack;
    this->_direction = direction;
    this->_specialAttack = specialAttack;
    return true;
}

float Unit::getAngleBetweenDirectionAndDefault() {
    int negativeY = _direction.y < 0 ? -1 : 1;
    return negativeY * acosf(Vec2::dot(_direction, getDefaultDirection()) / (_direction.length() * getDefaultDirection().length()));
}

vector<cugl::Vec2> Unit::getBasicAttackRotated() {
    vector<cugl::Vec2> result = _basicAttack;
    std::transform(result.begin(), result.end(), result.begin(), [&](Vec2 vec) { vec.rotate(getAngleBetweenDirectionAndDefault()); return Vec2(round(vec.x), round(vec.y)); });
    
    return result;
}

vector<cugl::Vec2> Unit::getSpecialAttackRotated() {
    vector<cugl::Vec2> result = _specialAttack;
    std::transform(result.begin(), result.end(), result.begin(), [&](Vec2 vec) { vec.rotate(getAngleBetweenDirectionAndDefault()); return Vec2(round(vec.x), round(vec.y)); });
    return result;
}