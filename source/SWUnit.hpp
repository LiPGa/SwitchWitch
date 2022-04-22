//
//  SWUnit.hpp
//  SwitchWitch
//
//  Created by Hedy Yang on 2/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWUnit_hpp
#define SWUnit_hpp

#include <cugl/cugl.h>
#include "SWSquareOccupant.hpp"
#include <math.h>

using namespace cugl;

/**
 * Model class representing an unit.
 */
class Unit : public SquareOccupant
{
public:
    /** Available colors for a unit. Each unit will have one of the three colors.*/
    enum Color
    {
        RED,
        GREEN,
        BLUE,
        NONE
    };
    
    enum State
    {
        IDLE,
        HIT
    };
    
    /** True if the unit has completed the animation cycle of its current state */
    bool completedAnimation = false;

private:
    /**Both the basic attack and special attack of a unit is stored as a list of vectors.
     * Each vector repesents a distance and direction of an attack. The direction is based on the
     * default direction, meaning that all attack data should be inputed assuming that the unit is facing
     * in the default direction.
     *
     * Attacks are assumed to kill any unit instantly.
     */

    /** The name of the unit subtype*/
    std::string _subtype;

    /** The basic attacks of this unit*/
    vector<cugl::Vec2> _basicAttack;

    /** The special attacks of this unit*/
    vector<cugl::Vec2> _specialAttack;

    /** The direction this unit is currently facing.*/
    cugl::Vec2 _direction;

    /** The color of this unit.*/
    Color _color;
    
    /** The state of this unit. */
    State _state = IDLE;
    
    /** This map stores a texture for every state the unit can take */
    unordered_map<std::string, shared_ptr<Texture>> _textureMap;

    /** The Sprite-Node that represents this unit */
    shared_ptr<cugl::scene2::SpriteNode> _viewNode;
    
    /** The elapsed time since the sprite frame was incremented */
    float _time_since_last_frame = 0.0f;
    
    /** The amout of time every sprite frame should remain for */
    float _time_per_frame = 0.5f;

    /**True if unit is a special unit, False otherwise*/
    bool _is_special_unit;

    /** The number of units needed to be killed in order to kill this unit. This is specifically used for the king unit. */
    int _unitsNeededToKill;

    /** Whether the unit is moveable */
    bool _moveable;

#pragma mark Constructors
public:
    /**
     * Creates an uninitialized Unit.
     *
     * You must initialize this Unit before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Unit on the
     * heap, use one of the static constructors instead.
     */
    Unit() {}

    /**
     * Disposes the unit, releasing all resources
     */
    ~Unit() {}

    /**
     * Initializes a unit given a color, direction, basic attack pattern, and special attack pattern.
     *
     * @param color the unit color
     * @param basicAttack the basic attack pattern of the unit
     * @param specialAttack the special attack pattern of the unit
     * @param direction the direction the unit is facing
     * @return true if initialization was successful.
     */
    bool init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction, bool moveable, bool special = false, int unitsNeededToKill = 0);


    bool init(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, cugl::Vec2 direction, bool moveable, bool special, int unitsNeededToKill);
    
    std::shared_ptr<cugl::Texture> getTextureForUnit(const std::string subtype, const Color color, const State state);
    
//    void initalizeTextureMap(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color);
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated unit.
     *
     * @param color the unit color
     * @param basicAttack the basic attack pattern of the unit
     * @param specialAttack the special attack pattern of the unit
     * @param direction the direction the unit is facing
     * @return a newly allocated Unit.
     */
    static std::shared_ptr<Unit>alloc(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, vector<cugl::Vec2> basicAttack, vector<cugl::Vec2> specialAttack, cugl::Vec2 direction, bool moveable, bool special = false, int unitsNeededToKill = 0) {
        std::shared_ptr<Unit> result = std::make_shared<Unit>();
        return (result->init(textures, subtype, color, basicAttack, specialAttack, direction, moveable, special, unitsNeededToKill) ? result : nullptr);
    }

    /**
     * Returns a newly allocated unit without the attack information
     *
     * @param color the unit color
     * @param direction the direction the unit is facing
     * @return a newly allocated Unit.
     */
    static std::shared_ptr<Unit>alloc(std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, const std::string subtype, const Color color, cugl::Vec2 direction, bool moveable, bool special = false, int unitsNeededToKill = 0) {
        std::shared_ptr<Unit> result = std::make_shared<Unit>();
        return (result->init(textures, subtype, color, direction, moveable, special, unitsNeededToKill) ? result : nullptr);
    }

#pragma mark -
#pragma mark Static Identifier
    /** The default direction of a unit.
     * This default direction repesents the direction the unit is assumed to be facing,
     * when inputing information reguarding the direction of attacks.
     * In other words, if the unit is facing in the default direction, all vectors
     * that represent the direction the unit should attack are not rotated.
     * If the unit is facing a direction different from the default direction,
     * all attack vectors are rotated so that an attack faces the direction of the unit.
     *
     * Currently, the default direction is Vec2(1,0). All units should have the same default direction.
     */
    static const cugl::Vec2 getDefaultDirection() { return Vec2::UNIT_X; }

    /**
     * Returns all possible directions the unit can face.
     *
     * @return a vector of all possible directions the unit can face in Vec2.
     */
    static const vector<cugl::Vec2> getAllPossibleDirections()
    {
        return vector<cugl::Vec2>{Vec2::UNIT_X, Vec2::UNIT_X * -1, Vec2::UNIT_Y, Vec2::UNIT_Y * -1};
    }

    /**
     * Returns the string data representation of the color according to JSON conventions
     *
     * @param c the color
     * @returns the string of the color
     */
    static std::string colorToString(Color c)
    {
        switch (c)
        {
        case Color::RED:
            return "red";
        case Color::GREEN:
            return "green";
        case Color::BLUE:
            return "blue";
        default:
            return "red";
        }
    }
    
    
    /**
     * Returns the string data representation of the state according to JSON conventions
     *
     * @param s the state
     * @returns the string of the state
     */
    static std::string stateToString(State s)
    {
        switch (s)
        {
        case State::IDLE:
            return "idle";
        default:
            return "idle";
        }
    }
    
    /**
     * Returns the string data representation of the direction
     *
     * @param c the direction
     * @returns the string of the direction
     */
    static std::string directionToString(cugl::Vec2 d)
    {
        if (d == Vec2(1,0)) {return "right";}
        else if (d == Vec2(-1,0)) {return "left";}
        else if (d== Vec2(0,1)) {return "up";}
        else {return "down";}
    }

    /**
     * Returns the color representation of a string that represents a color according to JSON conventions
     *
     * @param str string representation of the color. Must be either "red", "green", or "blue".
     * @returns the color
     */
    static Unit::Color stringToColor(std::string str)
    {
        if (str == "blue")
        {
            return Color::BLUE;
        }
        else if (str == "green")
        {
            return Color::GREEN;
        }
        else
        {
            return Color::RED;
        }
    }

#pragma mark -
#pragma mark Identifiers
    /**
     * Returns the subtype of the unit as a string.
     *
     * @return unit's subtype.
     */
    std::string getSubType() { return _subtype; }

    /**
     * Sets the subtype of the unit as a string.
     *
     * @return unit's subtype.
     */
    void setSubType(std::string subtype) { _subtype = subtype; }

    /**
     * Returns a list of vec2 representing all attacks
     * in a basic attack. The direction of attacks is based on the units default direction.
     *
     * @return unit's basic attack pattern
     */
    vector<cugl::Vec2> getBasicAttack() { return _basicAttack; }

    /**
     * Returns a list of vec2 representing all attacks in a basic attack
     * allready rotated based on direction the unit is facing.
     * The direction of attacks is based on the units default direction.
     *
     * @return unit's basic attack pattern already rotated.
     */
    vector<cugl::Vec2> getBasicAttackRotated();

    /**
     * Sets the unit's basic attack. The basic attack is represented as a list of vec2 representing
     * the direction and distance of an attack from its square.
     * The attack pattern should assume that the unit is facing in the default direction.
     *
     * @param attack the basic attack pattern of this unit.
     */
    void setBasicAttack(vector<cugl::Vec2> attack) { _basicAttack = attack; }

    /**
     * * Returns a list of vec2 representing all attacks
     * in a special attack. The direction of attacks is based on the units default direction.
     *
     * @return unit's special attack pattern
     */
    vector<cugl::Vec2> getSpecialAttack() { return _specialAttack; }

    /**
     * Returns a list of vec2 representing all attacks in a special attack
     * allready rotated based on direction the unit is facing.
     * The direction of attacks is based on the units default direction.
     *
     * @return unit's special attack pattern already rotated.
     */
    vector<cugl::Vec2> getSpecialAttackRotated();

    /**
     * Sets the unit's special attack. The special attack is represented as a list of vec2 representing
     * the direction and distance of an attack from its square.
     * The attack pattern should assume that the unit is facing in the default direction.
     *
     * @param attack the special attack pattern of this unit
     */
    void setSpecialAttack(vector<cugl::Vec2> attack) { _specialAttack = attack; }

    /**
     * Returns the unit's color as a Color enum.
     * @return unit's color
     */
    Color getColor() { return _color; }

    /**
     * Sets the unit's color
     *
     * @param c the color of the unit.
     */
    void setColor(Color c) { _color = c; }
    
    /**
     * Returns the unit's state as a State enum.
     * @return unit's state
     */
    State getState() { return _state; }

    /**
     * Sets the unit's state
     *
     * @param s the state of the unit.
     */
    void setState(State s);

    /**
     * Returns the direction which the unit is currently facing.
     *
     * @return unit's direction
     */
    cugl::Vec2 getDirection() { return _direction; }

    /**
     * Sets the unit's current direction.
     * Must be a unit vector that represents one of the 4 cardinal directions.
     * @param d the direction the unit is facing.
     */
    void setDirection(cugl::Vec2 d) { _direction = d; }

    /**
     * Retuns the angle between the direction of the unit and the default direction in radians.
     *
     * @returns the angle between direction and default direction
     */
    float getAngleBetweenDirectionAndDefault();

    /**
     * Returns the Sprite-Node that represents the unit's view.
     *
     * @return unit's Sprite-Node
     */
    shared_ptr<cugl::scene2::SpriteNode> getViewNode() { return _viewNode; }
    
    /**
     * Sets the Sprite-Node that represents the unit's view.
     *
     * @param a sprite-node for the unit.
     */
    void setViewNode(shared_ptr<cugl::scene2::SpriteNode> viewNode) {
        _viewNode = viewNode;
//        _time_since_last_frame = 0.0f;
    }

    /**
    Returns true if unit is a special unit
    */
    bool isSpecial() { return _is_special_unit; };

    /**
    Sets bool is a unit is special or not
    @special     bool of whether or not a unit is special
    */
    void setSpecial(bool special) { _is_special_unit = special; };

    bool isMoveable() { return _moveable; }

    void setMoveable(bool moveable) { _moveable = moveable; }

    int getUnitsNeededToKill() { return _unitsNeededToKill; }

    void setUnitsNeededToKill(int numOfUnits) { _unitsNeededToKill = numOfUnits; }
    
    /**
        Updates the animation frame of this unit.
     */
    void update(float dt);
};

#endif /* SWUnit_hpp */
