//
//  SWLevelEditorScene.cpp
//  Switch Witch
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This class is a reimagining of
//  the first game lab from 3152 in CUGL.
//
//  Based on Ship Lab
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <algorithm>

#include "SWLevelEditorScene.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** How big the unit radius should be */
#define UNIT_SIZE 50

/** How big the square width should be */
#define SQUARE_SIZE 128

/** How big the board is*/
#define BOARD_SIZE 5

#pragma mark Asset Constants

#pragma mark -
#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool LevelEditorScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
    {
        return false;
    }

    // Initialize game constants
    _constants = assets->get<JsonValue>("constants");
    _boardMembers = assets->get<JsonValue>("boardMember");
    _sceneHeight = _constants->getInt("scene-height");
    _boardSize = _constants->getInt("board-size");
    _squareSize = _constants->getInt("square-size");

    _assets = assets;

    dimen *= _sceneHeight / dimen.height;

    if (!Scene2::init(dimen)) {
        return false;
    }

    // TODO: JSON THIS AND MAKE IT MORE SCALABLE
    // UNIT ATTACK PATTERNS

    // Start up the input handler
    _input.init();

    // Get Textures
    // preload all the textures into a hashmap
    vector<string> textureVec = _constants->get("textures")->asStringArray();
    for (string textureName : textureVec) {
        _textures.insert({ textureName, _assets->get<Texture>(textureName) });
    }

    // layout
    _layout = scene2::AnchoredLayout::alloc();

    // set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());

    // Initialize state
    _currentState = SELECTING_UNIT;

    // Initialize Board
    _board = Board::alloc(BOARD_SIZE, BOARD_SIZE);

    // Set the view of the board.
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE));
    //_boardNode->setPosition(getSize() / 2);
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, 0));
    _boardNode->setTexture(_textures.at("transparent"));
    _board->setViewNode(_boardNode);

    _guiNode->addChildWithName(_boardNode, "boardNode");


    // initialize units with different types
    auto children = _boardMembers->get("unit")->children();
    for (auto child : children) {
        // get color
        string color = child->getString("color");
        Unit::Color c;
        if (color == "red") {
            c = Unit::Color::RED;
        }
        else if (color == "blue") {
            c = Unit::Color::BLUE;
        }
        else {
            c = Unit::Color::GREEN;
        }

        // get basic attack
        auto basicAttack = child->get("basic-attack")->asFloatArray();
        auto basicAttackVec = vector<Vec2>{ Vec2(basicAttack.at(0), basicAttack.at(1)) };

        // get special attack for this unit
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson) {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        shared_ptr<Unit> unit = Unit::alloc(c, basicAttackVec, specialAttackVec, Vec2(0, -1));
        _unitTypes.insert({ child->getString("texture"), unit });
    }

    // Create the squares & units and put them in the map
    for (int i = 0; i < _boardSize; i++) {
        for (int j = 0; j < _boardSize; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = (Vec2(i, j));
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            // Add square node to board node.
            _board->getViewNode()->addChild(squareNode);
            /*
            // generate unit for this square
        std:string unitType = unitsInBoard.at(_boardSize * (_boardSize - j - 1) + i);
            Vec2 unitDirection = unitsDirInBoard.at(_boardSize * (_boardSize - j - 1) + i);
            auto unitTemplate = _unitTypes.at(unitType);
            shared_ptr<Unit> unit = Unit::alloc(unitTemplate->getColor(), unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection);
            sq->setUnit(unit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitType));
            unit->setViewNode(unitNode);
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
            */
        }
    }
    reset();
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LevelEditorScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LevelEditorScene::update(float timestep)
{
    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    Vec2 pos = _input.getPosition();
    Vec2 boardPos = _boardNode->worldToNodeCoords(pos);
    /*
     * The mouse position is measured with the top left of the screen being the origin
     * while the origin of the board is on the bottom left.
     * The extra calculation on y is meant to convert the mouse position as if its origin is on the bottom left.
     */
    Vec2 squarePos = Vec2(int(boardPos.x) / SQUARE_SIZE, BOARD_SIZE - 1 - (int(boardPos.y) / SQUARE_SIZE));
    if (_board->doesSqaureExist(squarePos))
    {
        auto squareOnMouse = _board->getSquare(squarePos);
        if (_input.isDown())
        {
            if (_currentState == SELECTING_UNIT)
            {
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
            }
        }
    }
    // Layout everything
    _layout->layout(_guiNode.get());
}

#pragma mark -
#pragma mark Rendering
/**
 * Draws all this scene to the given SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch.  By overriding it, you can do custom drawing
 * in its place.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void LevelEditorScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch)
{
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    std::shared_ptr<Vec2> commonOffset = make_shared<Vec2>(getSize() / 2);
    _guiNode->render(batch);
    batch->end();
}
