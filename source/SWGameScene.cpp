//
//  SWGameScene.cpp
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

#include "SWGameScene.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** How big the square width should be */
#define SQUARE_SIZE 128

#pragma mark Asset Constants

#pragma mark -
#pragma mark Constructors

/**
 * sets the correct square texture for a unit.
 * @param square         the square being set
 * @param textures       the texture map being used
 */
void updateSquareTexture(shared_ptr<Square> square, std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures)
{
    if (square->getUnit()->isSpecial())
    {
        if (square->getUnit()->getDirection() == Vec2(0, 1))
        {
            square->getViewNode()->setTexture(textures.at("special_up_square"));
        }
        else if (square->getUnit()->getDirection() == Vec2(0, -1))
        {
            square->getViewNode()->setTexture(textures.at("special_down_square"));
        }
        else if (square->getUnit()->getDirection() == Vec2(1, 0))
        {
            square->getViewNode()->setTexture(textures.at("special_right_square"));
        }
        else
        {
            square->getViewNode()->setTexture(textures.at("special_left_square"));
        }
    }
    else
    {
        square->getViewNode()->setTexture(textures.at("square"));
    }
}

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    _debug = false;
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
        return false;

    // GetJSONValuesFromAssets
    _constants = assets->get<JsonValue>("constants");
    _boardMembers = assets->get<JsonValue>("boardMember");
    _boardJson = assets->get<JsonValue>("board");

    // Initialize Constants
    _sceneHeight = _constants->getInt("scene-height");
    vector<int> boardSize = _constants->get("board-size")->asIntArray();
    _boardWidth = boardSize.at(0);
    _boardHeight = boardSize.at(1);
    _squareSizeAdjustedForScale = _constants->getInt("square-size");
   

    // Initialize Scene
    dimen *= _sceneHeight / dimen.height;
    if (!Scene2::init(dimen))
        return false;

    // Start up the input handler
    _input.init();

    // Initialize Variables
    didRestart = false;
    _assets = assets;
    _score = 0;
    _prev_score = 0;

    // Get Textures
    // Preload all the textures into a hashmap
    vector<string> textureVec = _constants->get("textures")->asStringArray();
    for (string textureName : textureVec)
    {
        _textures.insert({textureName, _assets->get<Texture>(textureName)});
    }

    // Get the background image and constant values
    _background = assets->get<Texture>("background");
    _backgroundNode = scene2::PolygonNode::allocWithTexture(_background);
    _scale = getSize() / _background->getSize();
    _backgroundNode->setScale(_scale);

    _topuibackground = assets->get<Texture>("top-ui-background");
    _topuibackgroundNode = scene2::PolygonNode::allocWithTexture(_topuibackground);
    _scale.set(getSize().width / _topuibackground->getSize().width, getSize().width / _topuibackground->getSize().width);
    _topuibackgroundNode->setScale(_scale);

    // Allocate Layout
    _layout = scene2::AnchoredLayout::alloc();

    // Set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());
    addChild(_guiNode);
    _guiNode->addChild(_backgroundNode);
    _backgroundNode->setAnchor(Vec2::ZERO);
    _layout->addAbsolute("top_ui_background", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(0, -(_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_topuibackgroundNode, "top_ui_background");

    // Initialize state
    _currentState = NOTHING;

    // Initialize Board
    _board = Board::alloc(_boardHeight, _boardWidth);
    _currLevel = _boardJson->getInt("id");
    _turns = _boardJson->getInt("total-swap-allowed");
    _max_turns = _boardJson->getInt("total-swap-allowed");
    // thresholds for the star system
    _onestar_threshold = _boardJson->getInt("one-star-condition");
    _twostar_threshold = _boardJson->getInt("two-star-condition");
    _threestar_threshold = _boardJson->getInt("three-star-condition");

    // Create and layout the turn meter
    std::string turnMsg = strtool::format("%d/%d", _turns, _max_turns);
    _turn_text = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->setScale(0.75);
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -1.275 * (_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_turn_text, "turn_text");

    // Create and layout the score meter
    std::string scoreMsg = strtool::format("Score %d", _score);
    _score_text = scene2::Label::allocWithText(scoreMsg, assets->get<Font>("pixel32"));
    _score_text->setScale(0.75);
    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -0.85 * (_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_score_text, "score_text");

    // Set the view of the board.
    _squareSizeAdjustedForScale = int(SQUARE_SIZE * _scale.width);
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _boardWidth * _squareSizeAdjustedForScale, _boardHeight * _squareSizeAdjustedForScale));
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(-.05, -.1));
    _boardNode->setTexture(_textures.at("transparent"));
    _board->setViewNode(_boardNode);
    _guiNode->addChildWithName(_boardNode, "boardNode");

    // Initialize units with different types
    // Children will be types "basic", "three-way", etc.
    auto children = _boardMembers->get("unit")->children();
    for (auto child : children)
    {
        // Get basic attack
        auto subtypeString = child->key();
        auto basicAttackJson = child->get("basic-attack")->children();
        vector<Vec2> basicAttackVec;
        for (auto basicAttack : basicAttackJson)
        {
            auto basicAttackArray = basicAttack->asFloatArray();
            basicAttackVec.push_back(Vec2(basicAttackArray.at(0), basicAttackArray.at(1)));
        }
        // Get special attack
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson)
        {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        // store the default color:red for this type of unit
        shared_ptr<Unit> unit = Unit::alloc(subtypeString, Unit::Color::RED, basicAttackVec, specialAttackVec, Vec2(0, -1));
        _unitTypes.insert({child->key(), unit});
    }

    // Get the sub-type and color of the unit for every unit in this level
    // Get the direction of the unit for every unit in this level and save in a vector
    auto unitsInBoardJson = _boardJson->get("board-members")->children();
    vector<Vec2> unitsDirInBoard;
    vector<vector<std::string>> unitsInBoard;
    for (auto child : unitsInBoardJson)
    {
        auto unitDirArray = child->get("direction")->asFloatArray();
        unitsDirInBoard.push_back(Vec2(unitDirArray.at(0), unitDirArray.at(1)));
        auto unitColor = child->getString("color");
        auto unitSubType = child->getString("sub-type");
        vector<std::string> info{unitSubType, unitColor};
        unitsInBoard.push_back(info);
    }

    // Create the squares & units and put them in the map
    for (int i = 0; i < _boardWidth; i++)
    {
        for (int j = 0; j < _boardHeight; ++j)
        {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = Vec2(i, j);

            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale * 1.2) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            squareNode->setScale(_scale.width);
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->addChild(squareNode);
            //            // Generate unit for this square
            auto unitSubType = unitsInBoard.at(_boardWidth * (_boardHeight - j - 1) + i).at(0);
            auto unitColor = unitsInBoard.at(_boardWidth * (_boardHeight - j - 1) + i).at(1);
        std:
            string unitPattern = getUnitType(unitSubType, unitColor);
            Vec2 unitDirection = unitsDirInBoard.at(_boardWidth * (_boardHeight - j - 1) + i);
            auto unitTemplate = _unitTypes.at(unitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            shared_ptr<Unit> unit = Unit::alloc(unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection);
            sq->setUnit(unit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitPattern));
            unit->setViewNode(unitNode);
            if (unitSubType != "basic")
            {
                unit->setSpecial(true);
            }
            else
            {
                unit->setSpecial(false);
            }
            // unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
            updateSquareTexture(sq, _textures);
        }
    }

    _resultLayout = assets->get<scene2::SceneNode>("result");
    _resultLayout->setContentSize(dimen);
    _resultLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_resultLayout);
    _resultLayout->setVisible(false);
    
    _score_number = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("result_board_number"));
    _star1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star1"));
    _star2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star2"));
    _star3 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star3"));
    _restartbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_restart"));
    _restartbutton->deactivate();
    _restartbutton->setDown(false);
    _restartbutton->addListener([this](const std::string& name, bool down) {
        CULog("pressed");
        if (down) {
            CULog("down");
            didRestart = true;
        }
    });

    
    
    return true;
}

/**
 * Get the pattern for a unit provided its type and color
 *
 */
std::string GameScene::getUnitType(std::string type, std::string color)
{
    return type + "-" + color;
}

/**
 * Check if the given position is safe to hold a special unit
 *
 * Among the 8 squares around a special unit, there can be at most one other special unit
 */
bool GameScene::isSafe(cugl::Vec2 pos, cugl::Vec2 specialPosition[])
{
    int count = 0;
    for (int i = 0; i < specialPosition->length(); i++)
    {
        // if the given position is already taken by another special unit
        if (pos == specialPosition[i])
        {
            return false;
        }

        // if there is any special unit among the 8 adjacent squares of the given position
        if (specialPosition[i].x >= pos.x - 1 && specialPosition[i].x <= pos.x + 1 && specialPosition[i].y >= pos.y - 1 && specialPosition[i].y <= pos.y + 1)
        {
            count++;
        }
    }
    if (count <= 1)
        return true;
    return false;
}

/**
 * Generate a unit with random color, attack, and direction on the given square.
 *
 * @param sq    The given square
 * @param squareNode    The given squareNode
 */
void GameScene::generateUnit(shared_ptr<Square> sq)
{
    // Set Unit Information
    auto unit = sq->getUnit();
    unit->setDirection(Unit::getAllPossibleDirections()[rand() % 4]);
    unit->setColor(Unit::Color(rand() % 3));
    // TODO: Probabilities need to be inbedded in JSON.
    // TODO: Relation between probabilities and unitSubTypes stored in some kind of data structure.
    int basicUnitSpawnProbabilityPrecentage = 90;
    int twoForwardAttackSpawnProbabilityPrecentage = 4;
    int threeWayAttackSpawnProbabilityPrecentage = 4;
    int diagonalAttackSpawnProbabilityPrecentage = 2;
    auto unitSelectRandomNumber = rand() % 100;
    std::string unitSubTypeSelected;
    if (unitSelectRandomNumber < basicUnitSpawnProbabilityPrecentage)
    {
        unitSubTypeSelected = "basic";
    }
    else if (unitSelectRandomNumber < basicUnitSpawnProbabilityPrecentage + twoForwardAttackSpawnProbabilityPrecentage)
    {
        unitSubTypeSelected = "two-forward";
    }
    else if (unitSelectRandomNumber < basicUnitSpawnProbabilityPrecentage + twoForwardAttackSpawnProbabilityPrecentage + threeWayAttackSpawnProbabilityPrecentage)
    {
        unitSubTypeSelected = "three-way";
    }
    else
    {
        unitSubTypeSelected = "diagonal";
    }
    auto unitSelected = _unitTypes[unitSubTypeSelected];
    unit->setSubType(unitSelected->getSubType());
    unit->setBasicAttack(unitSelected->getBasicAttack());
    unit->setSpecialAttack(unitSelected->getSpecialAttack());

    // Update Unit Node
    auto unitNode = unit->getViewNode();
    unitNode->setTexture(_textures[getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()))]);
    // unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
    if (unitSubTypeSelected != "basic")
    {
        unit->setSpecial(true);
    }
    else
    {
        unit->setSpecial(false);
    }
    updateSquareTexture(sq, _textures);
}

/**
 * Upgrade a basic unit to a special unit.
 *
 * @param sq    The given square
 */
/*
void GameScene::upgradeToSpecial(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode) {

    auto unit = sq->getUnit();
    unit->setSpecial(true);
    auto unitNode = unit->getViewNode();
    auto randomNumber3 = rand() % 10 + 1;
    if (unit->getDirection() == Vec2(0, 1)) {
        sq->getViewNode()->setTexture(_textures.at("special_up_square"));
    }
    else if (unit->getDirection() == Vec2(0, -1)) {
        sq->getViewNode()->setTexture(_textures.at("special_down_square"));
    }
    else if (unit->getDirection() == Vec2(1, 0)) {
        sq->getViewNode()->setTexture(_textures.at("special_right_square"));
    }
    else {
        sq->getViewNode()->setTexture(_textures.at("special_left_square"));
    }
    //sq->getViewNode()->setTexture(_textures.at("special_up_square"));
    if (randomNumber3 <= 4) {
        unit->setSpecialAttack(twoForwardAttack);
    } else if (randomNumber3 > 4 && randomNumber3 <= 8) {
           unit->setSpecialAttack(threeWayAttack);
    } else {
           unit->setSpecialAttack(diagonalAttack);
    }

    if (unit->getColor() == Unit::RED) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_textures.at("two-forward-red"));
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_textures.at("three-way-red"));
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_textures.at("diagonal-red"));
        }
    } else if (unit->getColor() == Unit::GREEN) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_textures.at("two-forward-green"));
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_textures.at("three-way-green"));
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_textures.at("diagonal-green"));
        }
    } else if (unit->getColor() == Unit::BLUE) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_textures.at("two-forward-blue"));
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_textures.at("three-way-blue"));
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_textures.at("diagonal-blue"));
        }
    }
}
*/

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    if (_active)
    {
        removeAllChildren();
        _restartbutton = nullptr;
        _active = false;
    }
}

#pragma mark -
#pragma mark Gameplay Handling
/**
 * Returns the score based on the units that have been attacked.
 *
 * The score = the # of units killed times the # of colors killed times the # of special units killed.
 *
 * @param colorNum     The number of colors killed
 * @param basicUnitsNum    The number of basic units killed
 * @param specialUnitsNum The number of special units killed
 * @return The score of this attack
 */
int GameScene::calculateScore(int colorNum, int basicUnitsNum, int specialUnitsNum)
{
    return colorNum * (basicUnitsNum + specialUnitsNum) * (specialUnitsNum > 0 ? specialUnitsNum : 1);
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep)
{
    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    if (_turns == 0 and didRestart == true){
        CULog("Reset");
        reset();
    }

    if (_turns == 0)
    {
        _restartbutton->activate();
        _score_number->setText(to_string(_score));
        if (_score < _onestar_threshold)
        {
            _star1->setTexture(_textures.at("star_empty"));
            _star2->setTexture(_textures.at("star_empty"));
            _star3->setTexture(_textures.at("star_empty"));
        }
        else if (_score >= _onestar_threshold && _score < _twostar_threshold)
        {
            _star1->setTexture(_textures.at("star_full"));
            _star2->setTexture(_textures.at("star_empty"));
            _star3->setTexture(_textures.at("star_empty"));
        }
        else if (_score >= _twostar_threshold && _score < _threestar_threshold)
        {
            _star1->setTexture(_textures.at("star_full"));
            _star2->setTexture(_textures.at("star_full"));
            _star3->setTexture(_textures.at("star_empty"));
        }
        else
        {
            _star1->setTexture(_textures.at("star_full"));
            _star2->setTexture(_textures.at("star_full"));
            _star3->setTexture(_textures.at("star_full"));
        }
        
        _resultLayout->setVisible(true);
        return;
    }
    Vec2 pos = _input.getPosition();
    Vec3 worldCoordinate = screenToWorldCoords(pos);
    Vec2 boardPos = _boardNode->worldToNodeCoords(worldCoordinate);
    Vec2 squarePos = Vec2(int(boardPos.x / (_squareSizeAdjustedForScale * 1.2)), int(boardPos.y / (_squareSizeAdjustedForScale*1.2)));
    if (_input.isDebugDown())
    {
        _debug = !_debug;
    }
    if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0)
    {
        auto squareOnMouse = _board->getSquare(squarePos);
        if (_input.isDown())
        {
            if (_currentState == SELECTING_UNIT)
            {
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
                _currentState = SELECTING_SWAP;
            }
            else if (_currentState == SELECTING_SWAP && squareOnMouse->getPosition().distance(_selectedSquare->getPosition()) == 1)
            {
                _attackedColorNum = 0;
                _attackedBasicNum = 0;
                _attackedSpecialNum = 0;

                _currentState = CONFIRM_SWAP;
                _swappingSquare = squareOnMouse;
                // Rotation and Swapping of Model
                // We do this so that we can show the attack preview, without changing the actual positional view of units.
                _selectedSquareOriginalDirection = _selectedSquare->getUnit()->getDirection();
                _swappingSquareOriginalDirection = _swappingSquare->getUnit()->getDirection();
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                squareOnMouse->getViewNode()->setTexture(_textures.at("square-swap"));
                _attackedSquares = _board->getAttackedSquares(_swappingSquare->getPosition());

                unordered_set<Unit::Color, hash<int>> attackedColors;

                for (shared_ptr<Square> attackedSquares : _attackedSquares)
                {
                    attackedSquares->getViewNode()->setTexture(_textures.at("square-attacked"));

                    auto attackedUnit = attackedSquares->getUnit();
                    attackedColors.insert(attackedUnit->getColor());

                    if (attackedUnit->getSpecialAttack().size() == 0)
                        _attackedBasicNum++;
                    else
                        _attackedSpecialNum++;
                }
                _attackedColorNum = (int)attackedColors.size();
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                _currentState = SELECTING_SWAP;
                // If we are de-confirming a swap, we must undo the swap.
                _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                _selectedSquare->getUnit()->setDirection(_selectedSquareOriginalDirection);
                _swappingSquare->getUnit()->setDirection(_swappingSquareOriginalDirection);
                for (shared_ptr<Square> squares : _board->getAllSquares())
                {
                    updateSquareTexture(squares, _textures);
                }
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
            }
        }
        else if (_input.didRelease())
        {
            for (shared_ptr<Square> squares : _board->getAllSquares())
            {
                updateSquareTexture(squares, _textures);
            }
            if (_currentState == CONFIRM_SWAP)
            {
                //  Because the units in the model where already swapped.
                auto swappedUnitNode = _selectedSquare->getUnit()->getViewNode();
                auto selectedUnitNode = _swappingSquare->getUnit()->getViewNode();
                // Rotate Units
                // swappedUnitNode->setAngle(_selectedSquare->getUnit()->getAngleBetweenDirectionAndDefault());
                // selectedUnitNode->setAngle(_swappingSquare->getUnit()->getAngleBetweenDirectionAndDefault());
                // Updating View
                _selectedSquare->getViewNode()->removeChild(selectedUnitNode);
                _swappingSquare->getViewNode()->removeChild(swappedUnitNode);
                _selectedSquare->getViewNode()->addChild(swappedUnitNode);
                _swappingSquare->getViewNode()->addChild(selectedUnitNode);

                // remove the attacked squares
                for (shared_ptr<Square> attackedSquare : _attackedSquares)
                {
                    generateUnit(attackedSquare);
                }

                _turns--;
                _prev_score = _score;
                _score += calculateScore(_attackedColorNum, _attackedBasicNum, _attackedSpecialNum);
            }
            _currentState = SELECTING_UNIT;
        }
    }
    // Update the score meter
    _score_text->setText(strtool::format("Score %d", _score), true);
    if ((_prev_score < 9 && _score > 9) || (_prev_score < 99 && _score > 99))
    {
        _layout->remove("score_text");
        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -0.85 * (_topuibackgroundNode->getSize().height)));
    }
    // Update the remaining turns
    _turn_text->setText(strtool::format("%d/%d", _turns, _max_turns));

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
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    _guiNode->render(batch);

    if (_debug)
    {
        batch->setColor(Color4::RED);
        std::string unitType = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->getSubType();
        std::string unitColor = _selectedSquare == NULL ? "" : Unit::colorToString(_selectedSquare->getUnit()->getColor());
        auto direction = _selectedSquare == NULL ? Vec2::ZERO : _selectedSquare->getUnit()->getDirection();
        std::ostringstream unitDirection;
        unitDirection << "(" << int(direction.x) << ", " << int(direction.y) << ")";
        batch->drawText(unitType, _turn_text->getFont(), Vec2(50, getSize().height - 100));
        batch->drawText(unitColor, _turn_text->getFont(), Vec2(50, getSize().height - 150));
        batch->drawText(unitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 200));
        std::string spe = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->isSpecial() ? "isSpecial() = true"
                                                                                                 : "isSpecial() = false";
        batch->drawText(spe, _turn_text->getFont(), Vec2(50, getSize().height - 250));
    }

    batch->end();
}
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset()
{
    //    _endgame_text->setForeground(Color4::CLEAR);
    //    _turns = _boardJson->getInt("total-swap-allowed");
    //    _score = 0;
    removeChild(_guiNode);
    init(_assets);
}

/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void GameScene::setActive(bool value)
{
    _active = value;
//        if (value && ! _restartbutton->isActive()) {
//            _restartbutton->activate();
//        } else if (!value && _restartbutton->isActive()) {
//            _restartbutton->deactivate();
//        }
}
