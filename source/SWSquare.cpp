#include "SWSquare.hpp"

using namespace cugl;

/**
 * Creats a square with the given position
 *
 * @param position of the square
 * @return if the initialization was successful
 */
bool Square::initWithPos(const cugl::Vec2& pos) {
	this->_pos = pos;
    return true;
}

/**
 * Creats a square with the given position and unit
 *
 * @param position of the square
 * @param unit on the square
 * @return if the initialization was successful
 */
bool Square::initWithPosAndUnit(const cugl::Vec2& pos, shared_ptr<Unit> unit) {
    this->_unit = unit;
    return initWithPos(pos);
}