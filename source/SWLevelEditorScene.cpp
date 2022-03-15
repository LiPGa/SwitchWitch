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
#define BOARD_WIDTH 5

#define BOARD_HEIGHT 5

#define SELECTION_BOARD_WIDTH 2
#define SELECTION_BOARD_HEIGHT 10

/** How to reduce the scale of textures for select*/
#define SELECTION_BOARD_SCALE 0.5

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
    if (assets == nullptr) return false;

    // GetJSONValuesFromAssets
    _constants = assets->get<JsonValue>("constants");
    _boardMembers = assets->get<JsonValue>("boardMember");
    _boardJson = assets->get<JsonValue>("board");

    // Initialize Constants
    _sceneHeight = _constants->getInt("scene-height");
    _boardSize = _constants->getInt("board-size");
    _squareSize = _constants->getInt("square-size");

    // Initialize Scene
    dimen *= _sceneHeight / dimen.height;
    if (!Scene2::init(dimen)) return false;

    // Start up the input handler
    _input.init();

    _currentState = State::NOTHING;

    // Get Textures
    // preload all the textures into a hashmap
    _assets = assets;
    vector<string> textureVec = _constants->get("textures")->asStringArray();
    for (string textureName : textureVec) {
        _textures.insert({ textureName, _assets->get<Texture>(textureName) });
    }

    // layout
    _layout = scene2::AnchoredLayout::alloc();

    // set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());

    // Initialize Board
    _board = Board::alloc(BOARD_HEIGHT, BOARD_WIDTH);

    /// Initialize Selection Board
    _selectionBoard = Board::alloc(SELECTION_BOARD_HEIGHT, SELECTION_BOARD_WIDTH);

    // Set the view of the board.
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, BOARD_WIDTH * SQUARE_SIZE, BOARD_HEIGHT * SQUARE_SIZE));
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, 0));
    _boardNode->setTexture(_textures.at("transparent"));
    _board->setViewNode(_boardNode);
    _guiNode->addChildWithName(_boardNode, "boardNode");

    // Set the view of the select board.
    _selectionBoardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, SQUARE_SIZE, BOARD_HEIGHT * SQUARE_SIZE));
    _layout->addRelative("selectionBoardNode", cugl::scene2::Layout::Anchor::MIDDLE_LEFT, Vec2(SQUARE_SIZE / getSize().width, 0));
    _selectionBoardNode->setTexture(_textures.at("transparent"));
    _selectionBoard->setViewNode(_selectionBoardNode);
    _guiNode->addChildWithName(_selectionBoardNode, "selectionBoardNode");

    // Initialize units with different types
    // Children will be types "basic", "three-way", etc.
    auto children = _boardMembers->get("unit")->children();
    for (auto child : children) {
        // Get basic attack
        auto subtypeString = child->key();
        auto basicAttackJson = child->get("basic-attack")->children();
        vector<Vec2> basicAttackVec;
        for (auto basicAttack : basicAttackJson) {
            auto basicAttackArray = basicAttack->asFloatArray();
            basicAttackVec.push_back(Vec2(basicAttackArray.at(0), basicAttackArray.at(1)));
        }
        // Get special attack
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson) {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        // store the default color:red for this type of unit
        shared_ptr<Unit> unit = Unit::alloc(subtypeString, Unit::Color::RED, basicAttackVec, specialAttackVec, Vec2(0, -1));
        _unitTypes.insert({ child->key(), unit });
    }

    // Create the squares & units and put them in the map
    for (int i = 0; i < _boardSize; i++) {
        for (int j = 0; j < _boardSize; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = (Vec2(i, j));
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->addChild(squareNode);
            // Generate unit for this square
            auto unitTemplateBeginning = _unitTypes.begin();
            auto unitSubType = unitTemplateBeginning->first;
            auto unitColor = "red";
            std:string unitPattern = getUnitType(unitSubType, unitColor);
            Vec2 unitDirection = Unit::getDefaultDirection();
            auto unitTemplate = _unitTypes.at(unitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            shared_ptr<Unit> unit = Unit::alloc(unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection);
            sq->setUnit(unit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitPattern));
            unit->setViewNode(unitNode);
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
        }
    }

    for (int i = 0; i < SELECTION_BOARD_WIDTH; i++) {
        for (int j = 0; j < SELECTION_BOARD_HEIGHT; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = (Vec2(i, j));
            squareNode->setScale(SELECTION_BOARD_SCALE);
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE * SELECTION_BOARD_SCALE) + Vec2::ONE * (SQUARE_SIZE * SELECTION_BOARD_SCALE / 2));
            shared_ptr<Square> sq = _selectionBoard->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _selectionBoard->getViewNode()->addChild(squareNode);
        }
    }
    auto i = 0;
    for each (auto element in _unitTypes) {
        auto square = _selectionBoard->getAllSquares()[i];
        auto unitType = element.second;        
        square->setUnit(unitType);
        auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(getUnitType(unitType->getSubType(), Unit::colorToString(unitType->getColor()))));
        square->getUnit()->setViewNode(unitNode);
        square->getViewNode()->addChild(unitNode);
        i++;
    }

    // Create the squares & units for the selection board and put them on the map.
    reset();
    return true;
}

/**
 * Get the pattern for a unit provided its type and color
 *
 */
std::string LevelEditorScene::getUnitType(std::string type, std::string color) {
    return type + "-" + color;
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
    Vec2 selectionBoardPos = _selectionBoardNode->worldToNodeCoords(pos);
    /*
     * The mouse position is measured with the top left of the screen being the origin
     * while the origin of the board is on the bottom left.
     * The extra calculation on y is meant to convert the mouse position as if its origin is on the bottom left.
     */
    
    if (_input.didPress())
    {
        Vec2 squarePos = Vec2(int(boardPos.x) / SQUARE_SIZE, BOARD_HEIGHT - 1 - (int(boardPos.y) / SQUARE_SIZE));
        Vec2 selectionSquarePos = Vec2(int(selectionBoardPos.x) / int(SQUARE_SIZE * SELECTION_BOARD_SCALE), SELECTION_BOARD_HEIGHT - 1 - (int(selectionBoardPos.y) / int(SQUARE_SIZE * SELECTION_BOARD_SCALE)));
        if (_board->doesSqaureExist(squarePos) && boardPos.x>=0 && boardPos.y>=0)
        {
            _currentState = State::CHANGING_BOARD;
            auto squareOnMouse = _board->getSquare(squarePos);
            if (_selectedSquare != squareOnMouse) {
                if (_selectedSquare != NULL) _selectedSquare->getViewNode()->setTexture(_textures.at("square"));
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
                if (_selectedUnitFromSelectionBoard != NULL) {
                    auto unit = _selectedSquare->getUnit();
                    auto unitThatWillReplace = _selectedUnitFromSelectionBoard->getUnit();
                    unit->setBasicAttack(unitThatWillReplace->getBasicAttack());
                    unit->setSpecialAttack(unitThatWillReplace->getSpecialAttack());
                    unit->setSubType(unitThatWillReplace->getSubType());
                }
            }
            else {
                _selectedSquare->getViewNode()->setTexture(_textures.at("square"));
                _selectedSquare = NULL;
            }
        }
        if (_selectionBoard->doesSqaureExist(selectionSquarePos) && selectionBoardPos.x >= 0 && selectionBoardPos.y >= 0) {
            _currentState = State::CHANGING_BOARD;
            auto squareOnMouse = _selectionBoard->getSquare(selectionSquarePos);
            if (_selectedUnitFromSelectionBoard != squareOnMouse) {
                if (_selectedUnitFromSelectionBoard != NULL) _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square"));
                _selectedUnitFromSelectionBoard = squareOnMouse;
                _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square-selected"));
            }
            else {
                _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square"));
                _selectedUnitFromSelectionBoard = NULL;
            }
        }    
    }
    if (_selectedSquare != NULL && State::CHANGING_BOARD) {
        auto unit = _selectedSquare->getUnit();
        if (_input.isDirectionKeyDown()) {
            unit->setDirection(_input.directionPressed());
            unit->getViewNode()->setAngle(unit->getAngleBetweenDirectionAndDefault());
        }
        if (_input.isRedDown()) {
            unit->setColor(Unit::RED);            
        }
        else if (_input.isGreenDown()) {
            unit->setColor(Unit::GREEN);
        }
        else if (_input.isBlueDown()) {
            unit->setColor(Unit::BLUE);
        }
        unit->getViewNode()->setTexture(_textures.at(getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()))));
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
