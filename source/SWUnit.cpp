//
//  SWUnit.cpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SWUnit.hpp"
using namespace cugl;

//void Unit::initalizeTextureMap(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, std::string subtype, Unit::Color color) {
//    std::string colorString = Unit::colorToString(color);
//    std::string defaultTextureName = subtype + "-" + colorString;
//    std::string idleTextureName = subtype + "-idle-" + colorString;
//    _textureMap[State::IDLE] = textures.count(idleTextureName) > 0 ? textures.at(idleTextureName) : textures.at(defaultTextureName);
//}

/**
 * Initializes a unit given a color, direction, basic attack pattern, and special attack pattern.
 *
 * @param color the unit color
 * @param basicAttack the basic attack pattern of the unit
 * @param specialAttack the special attack pattern of the unit
 * @param direction the direction the unit is facing
 * @return true if initialization was successful.
 */
bool Unit::init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, cugl::Vec2 direction, bool moveable, bool special, int unitsNeededToKill)
{
    this->_subtype = subtype;
    this->_color = color;
    this->_direction = direction;
    this->_is_special_unit = special;
    this->_moveable = moveable;
    this->_unitsNeededToKill = unitsNeededToKill;
    this->_textureMap = textures;
    this->_hasBeenHit = false;
//    this->_time_since_last_frame = fmod(((float) rand() / (RAND_MAX)), this->_time_per_frame); // Initalize a random animation offset
//    initalizeTextureMap(textures, subtype, color);
    return true;
}

/**
 * Initializes a unit given a color, direction, basic attack pattern, and special attack pattern.
 *
 * @param color the unit color
 * @param basicAttack the basic attack pattern of the unit
 * @param specialAttack the special attack pattern of the unit
 * @param direction the direction the unit is facing
 * @return true if initialization was successful.
 */
bool Unit::init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction, bool moveable, bool special, int unitsNeededToKill)
{
    this->_subtype = subtype;
    this->_color = color;
    this->_basicAttack = basicAttack;
    this->_direction = direction;
    this->_specialAttack = specialAttack;
    this->_moveable = moveable;
    this->_is_special_unit = special;
    this->_unitsNeededToKill = unitsNeededToKill;
    this->_textureMap = textures;
    this->_hasBeenHit = false;
//    this->_time_since_last_frame = fmod(((float) rand() / (RAND_MAX)), this->_time_per_frame); // Initalize a random animation offset
//    initalizeTextureMap(textures, subtype, color);
    return true;
}

std::shared_ptr<cugl::Texture> Unit::getTextureForUnit(const std::string subtype, const Color color, const State state) {
    std::string colorString = Unit::colorToString(color);
    std::string defaultTextureName = subtype + "-" + colorString;
    std::string idleTextureName = subtype + "-idle-" + colorString;
    std::string hitTextureName = subtype + "-hit-" + colorString;
    std::string attackTextureName = subtype + "-attack-" + colorString;
    std::string dyingTextureName = subtype + "-dying-" + colorString;
    std::string respawningTextureName = subtype + "-respawning-" + colorString;
    switch (state) {
        case IDLE:
            return _textureMap.count(idleTextureName) > 0 ? _textureMap.at(idleTextureName) : _textureMap.at(defaultTextureName);
        case HIT:
            return _textureMap.count(hitTextureName) > 0 ? _textureMap.at(hitTextureName)
            : _textureMap.at(defaultTextureName);
        case ATTACKING:
            return _textureMap.count(attackTextureName) > 0 ? _textureMap.at(attackTextureName) : _textureMap.at(defaultTextureName);
        case DYING:
            return _textureMap.count(dyingTextureName) > 0 ? _textureMap.at(dyingTextureName) : _textureMap.at(defaultTextureName);
        case DEAD:
            return _textureMap.at("transparent");
        case RESPAWNING:
            return _textureMap.count(respawningTextureName) > 0 ? _textureMap.at(respawningTextureName) : _textureMap.at(defaultTextureName);
        default:
            return _textureMap.at(defaultTextureName);
    }
}



void Unit::setState(State s) {
//    if (s == _state) return;
    _state = s;
    std::shared_ptr<scene2::SpriteNode> newNode;
    completedAnimation = false;
    switch (s) {
        case State::IDLE:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 2);
            break;
        case State::PROTECTED:
            newNode = scene2::SpriteNode::alloc(_textureMap.at("shield"), 1, 1);
            break;
        case State::HIT:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 1);
            this->_hasBeenHit = true;
            break;
        case State::ATTACKING:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 2);
            break;
        case State::DYING:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 2);
            break;
        case State::DEAD:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 1);
            break;
        case State::RESPAWNING:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 2);
            break;
        default:
            newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, 1);
            
    }
    _viewNode = newNode;
}

/**
 * Retuns the angle between the direction of the unit and the default direction in radians.
 *
 * @returns the angle between direction and default direction
 */
float Unit::getAngleBetweenDirectionAndDefault()
{
    int negativeY = _direction.y < 0 ? -1 : 1;
    return negativeY * acosf(Vec2::dot(_direction, getDefaultDirection()) / (_direction.length() * getDefaultDirection().length()));
}

/**
 * Returns a list of vec2 representing all attacks in a basic attack
 * allready rotated based on direction the unit is facing.
 * The direction of attacks is based on the units default direction.
 *
 * @return unit's basic attack pattern already rotated.
 */
vector<cugl::Vec2> Unit::getBasicAttackRotated()
{
    vector<cugl::Vec2> result = _basicAttack;
    std::transform(result.begin(), result.end(), result.begin(), [&](Vec2 vec)
                   { vec.rotate(getAngleBetweenDirectionAndDefault()); return Vec2(round(vec.x), round(vec.y)); });

    return result;
}

/**
 * Returns a list of vec2 representing all attacks in a special attack
 * allready rotated based on direction the unit is facing.
 * The direction of attacks is based on the units default direction.
 *
 * @return unit's special attack pattern already rotated.
 */
vector<cugl::Vec2> Unit::getSpecialAttackRotated()
{
    vector<cugl::Vec2> result = _specialAttack;
    std::transform(result.begin(), result.end(), result.begin(), [&](Vec2 vec)
                   { vec.rotate(getAngleBetweenDirectionAndDefault()); return Vec2(round(vec.x), round(vec.y)); });
    return result;
}

void Unit::update(float dt) {
//    auto* _spriteNode = dynamic_cast<cugl::scene2::SpriteNode*>(_viewNode.get());
//    if (_spriteNode) {
    _time_since_last_frame += dt;
    if (_time_since_last_frame > _time_per_frame) {
        _time_since_last_frame = 0.0f;
        int frame = _viewNode->getFrame() + 1;
        if (frame >= _viewNode->getSize()) {
            frame = 0;
            completedAnimation = true;
        }
        _viewNode->setFrame(frame);
    }
//    }
}
