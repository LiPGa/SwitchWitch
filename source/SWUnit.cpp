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
    // <Hedy>
    std::string selectedStartTextureName = subtype + "-" + colorString + "-selected-start";
    std::string selectedEndTextureName = subtype + "-" + colorString + "-selected-end";
    // <Hedy/>
    std::string targetedTextureName = subtype + "-target-" + colorString;
//    CULog("targetedTexture: %s", targetedTextureName.c_str());
    std::string attackTextureName = subtype + "-attack-" + colorString;
    std::string dyingTextureName = subtype + "-dying-" + colorString;
    std::string respawningTextureName = subtype + "-respawning-" + colorString;
    switch (state) {
        case IDLE:
            return _textureMap.count(idleTextureName) > 0 ? _textureMap.at(idleTextureName) : _textureMap.at(defaultTextureName);
        case PROTECTED:
            return _textureMap.count(idleTextureName) > 0 ? _textureMap.at(idleTextureName) : _textureMap.at(defaultTextureName);
        case HIT:
            return _textureMap.count(hitTextureName) > 0 ? _textureMap.at(hitTextureName)
            : _textureMap.at(defaultTextureName);
        // <Hedy>
        case SELECTED_START:
            return _textureMap.count(selectedStartTextureName) > 0 ? _textureMap.at(selectedStartTextureName) : _textureMap.at(defaultTextureName);
        case SELECTED_END:
            return _textureMap.count(selectedEndTextureName) > 0 ? _textureMap.at(selectedEndTextureName) : _textureMap.at(defaultTextureName);
        // <Hedy/>
        case TARGETED:
            return _textureMap.count(targetedTextureName) > 0 ? _textureMap.at(targetedTextureName) : _textureMap.at(defaultTextureName);
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
    _state = s;
    //<Hedy>
    if (s != State::SELECTED_MOVING && s != State::SELECTED_NONE) {
    // moving animation is a special case
    completedAnimation = false;
    // <Hedy/>
    std::shared_ptr<scene2::SpriteNode> newNode;
    int framesInAnimation = animationFrameCounts[s];
//    if (_subtype == "king" && s == DYING) framesInAnimation = 5; // King dying animation is a special case
    if (_subtype == "basic" && s == ATTACKING) {
        framesInAnimation = 1; // Basic attack animation is a special case
        _initialPos = _viewNode->getPosition();
    }
    newNode = scene2::SpriteNode::alloc(getTextureForUnit(this->_subtype, this->_color, s), 1, framesInAnimation);
    if (s == State::HIT) _hasBeenHit = true;
    if (s == State::PROTECTED) {
        auto shieldNode = scene2::SpriteNode::alloc(_textureMap.at("shield"),1,1);
//            std::shared_ptr<cugl::scene2::AnchoredLayout> unitLayout = scene2::AnchoredLayout::alloc();
        shieldNode->setScale(1 / _viewNode->getScale());
        shieldNode->setAnchor(Vec2::ANCHOR_CENTER);
        shieldNode->setPosition(Vec2(newNode->getWidth() / 2.0f, newNode->getHeight() / 2.0f));
//            newNode->setAnchor(Vec2::ANCHOR_CENTER);
        newNode->addChild(shieldNode);
//            unitLayout->add
            
    }
    if (s == IDLE) { // Speed up the animation if part of a chain
        _time_per_animation = DEFAULT_TIME_PER_ANIMATION;
        _chainCount = 0;
    } else if (_subtype != "king") {
        _time_per_animation = std::max(0.2f, DEFAULT_TIME_PER_ANIMATION - _chainCount * ANIMATION_SPEEDUP_FACTOR);
    }
//    if (s == State::TARGETED) {
//        CULog("targeted frames in animation: %i", framesInAnimation);
//        framesInAnimation = 4;
//    }
    _time_per_frame = _time_per_animation / framesInAnimation;
    _time_since_last_flash = 0.0f;
    _time_since_last_frame = 0.0f;
    _time_since_start_animation = 0.0f;
    newNode->setAnchor(Vec2::ANCHOR_CENTER + Vec2(0, -0.2));
    newNode->setVisible(true);
//    newNode->setLayout()
    _viewNode = newNode;
    }
}

//<Hedy>
/**
 * Sets the selected_end animation
 *
 * @param texture the texture of the SELECTED unit
 *
 */
void Unit::setSelectedEnd(std::shared_ptr<cugl::Texture> texture)
{
    _state = SELECTED_END;
    completedAnimation = false;
    std::shared_ptr<scene2::SpriteNode> newNode;
    int framesInAnimation = animationFrameCounts[SELECTED_END];
    newNode = scene2::SpriteNode::alloc(texture, 1, framesInAnimation);
    _time_per_frame = _time_per_animation / framesInAnimation;
    _time_since_last_flash = 0.0f;
    _time_since_last_frame = 0.0f;
    _time_since_start_animation = 0.0f;
    newNode->setAnchor(Vec2::ANCHOR_CENTER + Vec2(0, -0.2));
    newNode->setVisible(true);
    _viewNode = newNode;
};
//<Hedy/>

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
    _time_since_last_flash += dt;
    _time_since_start_animation += dt;
    switch (_state) { // state-specific effects
        case HIT: { // flashing effect
            float timePerFlash = _time_per_animation / 2 / _num_flashes;
            if (_time_since_last_flash > timePerFlash) {
                _time_since_last_flash = 0.0f;
                _viewNode->setVisible(!_viewNode->isVisible());
            }
            break;
        }
        case ATTACKING: {
            if (_subtype == "basic") { // basic units have a special attack effect
                float timeBeforeMovement = _time_per_animation / 4.0f; // begin the movement effect after 1/4 animation is complete
                if (_time_since_start_animation <= _time_per_animation / 2.0f && _time_since_start_animation >= timeBeforeMovement) {
                    Vec2 newPos = _initialPos + _direction * _basicAttackDistance * ((_time_since_start_animation - timeBeforeMovement) / timeBeforeMovement);
                    _viewNode->setPosition(newPos);
                } else if (_time_since_start_animation <= 3.0f * timeBeforeMovement && _time_since_start_animation >= _time_per_animation / 2.0f) {
                    Vec2 newPos = _initialPos + _direction * _basicAttackDistance * (1 - ((_time_since_start_animation - 2.0f * timeBeforeMovement) / timeBeforeMovement));
                    _viewNode->setPosition(newPos);
                }
            }
            break;
        }
//        case DYING: {
//            if (_subtype == "king") break; // King dying animation is a special case
//            Color4 newColor = Color4::CLEAR;
//            float cols[] = { 0.0, 0.0, 0.0, std::max(0.0f, 1 - (_time_since_start_animation / _time_per_animation)) };
//            newColor.set(cols);
//            _viewNode->setColor(newColor);
//            break;
//        }
        case RESPAWNING: {
//            _viewNode->setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            float cols[] = { 1.0, 1.0, 1.0, std::min(1.0f, (_time_since_start_animation / _time_per_animation)) };
            Color4 newColor = Color4(cols);
            _viewNode->setColor(newColor);
            break;
        }
        default: {
            break;
        }
    }
    if (_time_since_last_frame > _time_per_frame) {
        _time_since_last_frame = 0.0f;
        int frame = _viewNode->getFrame() + 1;
        if (frame >= _viewNode->getSize()) {
            if (animationShouldLoop(_state)) frame = 0;
            else frame = _viewNode->getSize() - 1;
            completedAnimation = true;
//            _viewNode->setPosition(_initialPos);
        }
        _viewNode->setFrame(frame);
    }
//    }
}

bool Unit::animationShouldLoop(State s) {
    switch (s) {
        case IDLE:
            return true;
        case TARGETED:
            return true;
        default:
            return false;
    }
}
