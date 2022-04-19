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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    _debug = false;
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
        return false;

    // GetJSONValuesFromAssets
    std::shared_ptr<cugl::JsonValue> constants = assets->get<JsonValue>("constants");
    std::shared_ptr<cugl::JsonValue> boardMembers = assets->get<JsonValue>("boardMember");

    // Initialize Constants
    auto sceneHeight = constants->getInt("scene-height");
    vector<int> boardSize = constants->get("board-size")->asIntArray();
    _boardWidth = boardSize.at(0);
    _boardHeight = boardSize.at(1);
    _defaultSquareSize = constants->getInt("square-size");

    // Initialize Scene
    dimen *= sceneHeight / dimen.height;
    if (!Scene2::init(dimen))
        return false;

    // Start up the input handler
    _input.init();

    // Initialize Variables
    _assets = assets;
    
    // Seed the random number generator to a new seed
    srand(static_cast<int>(time(NULL)));


    _didRestart = false;
    _didGoToLevelMap = false;
    _didPause = false;
    // Get Textures
    // Preload all the textures into a hashmap
    vector<string> textureVec = constants->get("textures")->asStringArray();
    for (string textureName : textureVec)
    {
        _textures.insert({textureName, _assets->get<Texture>(textureName)});
    }
    
    // Get Probablities
    // Preload all the probabilities into a hashmap
    vector<string> probabilityVec = constants->get("probability-respawn")->asStringArray();
    int probabilitySum = 0;
    for (string probabilityName : probabilityVec)
    {
        int respawnProb = boardMembers-> get("unit") -> get(probabilityName) -> get("probability-respawn") -> asInt();
        probabilitySum += respawnProb;
        _probability.insert({probabilityName, probabilitySum});
        _unitRespawnProbabilities.insert({probabilityName, static_cast<float>(respawnProb)});
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

    // Create and layout the turn meter
    std::string turnMsg = strtool::format("%d/%d", _turns, 100);
    _turn_text = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->setScale(0.75);
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -1.275 * (_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_turn_text, "turn_text");
//
//    // Create and layout unitsNeededToKill info
//    auto units_needed_to_kill = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    

    // Create and layout the score meter

    _scoreMeter = scene2::ProgressBar::allocWithCaps(_assets->get<Texture>("scoremeter_background"), _assets->get<Texture>("scoremeter_foreground"), _assets->get<Texture>("scoremeter_endcap_left"), _assets->get<Texture>("scoremeter_endcap_right"), Size(97, 15));
    _scoreMeter->setProgress(0.0);
//    _scoreMeter->setPosition(Vec2(0, 0));
    _layout->addAbsolute("scoreMeter", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(10 -_topuibackgroundNode->getSize().width / 6, -0.87 * (_topuibackgroundNode->getSize().height)));
//    _layout->addRelative("scoreMeter", cugl::scene2::Layout::Anchor::CENTER_FILL, Vec2(0, 0));
    _guiNode->addChildWithName(_scoreMeter, "scoreMeter");
    _scoreMeterStar1 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
    _scoreMeterStar1->setScale(0.15f);
    _scoreMeterStar2 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
    _scoreMeterStar2->setScale(0.15f);
    _scoreMeterStar3 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
    _scoreMeterStar3->setScale(0.15f);
    
    _guiNode->addChildWithName(_scoreMeterStar1, "scoreMeterStar1");
    _guiNode->addChildWithName(_scoreMeterStar2, "scoreMeterStar2");
    _guiNode->addChildWithName(_scoreMeterStar3, "scoreMeterStar3");
    std::string scoreMsg = strtool::format("%d", _score);
    _score_text = scene2::Label::allocWithText(scoreMsg, assets->get<Font>("pixel32"));
    _score_text->setScale(0.35);
    _score_text->setHorizontalAlignment(HorizontalAlign::RIGHT);
    _score_text->setAnchor(Vec2(1.0, 0.5));
    _score_text->setForeground(Color4::WHITE);
    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + 12,
                                                                                      _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
//    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_LEFT, Vec2(20, 20));
    _guiNode->addChildWithName(_score_text, "score_text");
    

    // Set the view of the board.
    CULog("%d %d", _scale.width, _scale.height);
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height);
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _boardWidth * _squareSizeAdjustedForScale, _boardHeight * _squareSizeAdjustedForScale));
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(-.05, -.1));
    _boardNode->setTexture(_textures.at("transparent"));
    _board->setViewNode(_boardNode);
    _guiNode->addChildWithName(_boardNode, "boardNode");

    // Initialize units with different types
    // Children will be types "basic", "three-way", etc.
    auto children = boardMembers->get("unit")->children();
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

    _resultLayout = assets->get<scene2::SceneNode>("result");
    _resultLayout->setContentSize(dimen);
    _resultLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_resultLayout);
    _resultLayout->setVisible(false);
    
    _settingsLayout = assets->get<scene2::SceneNode>("settings");
    _settingsLayout->setContentSize(dimen);
    _settingsLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_settingsLayout);
    
    _settingsMenuLayout = assets->get<scene2::SceneNode>("settings-menu");
    _settingsMenuLayout->setContentSize(dimen);
    _settingsMenuLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_settingsMenuLayout);
    _settingsMenuLayout->setVisible(false);
    
    _score_number = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("result_board_number"));
    _star1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star1"));
    _star2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star2"));
    _star3 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star3"));
    _restartbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_restart"));
    _restartbutton->deactivate();
    _restartbutton->setDown(false);
    _restartbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didRestart = true;
        }
    });
    
    _settingsbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_btn"));
    _settingsbutton->setVisible(true);
    _settingsbutton->activate();
    _settingsbutton->setDown(false);
    _settingsbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didPause = true;
            CULog("Pressed Pause/Settings button");
        }
    });
    
    _settingsRestartBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_restart"));
    _settingsRestartBtn->setVisible(true);
    _settingsRestartBtn->deactivate();
    _settingsRestartBtn->setDown(false);
    _settingsRestartBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didRestart = true;
            CULog("Pressed Settings restart button");
        }
    });
    
    _settingsCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_close"));
    _settingsCloseBtn->setVisible(true);
    _settingsCloseBtn->deactivate();
    _settingsCloseBtn->setDown(false);
    _settingsCloseBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didPause = false;
            _settingsMenuLayout->setVisible(false);
            CULog("Pressed Settings restart button");
        }
    });
    
    _settingsBackBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_exit"));
    _settingsBackBtn->setVisible(true);
    _settingsBackBtn->deactivate();
    _settingsBackBtn->setDown(false);
    _settingsBackBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didGoToLevelMap = true;
            CULog("Pressed Settings exit button");
        }
    });

    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_exit"));
    _backbutton->deactivate();
    _backbutton->setDown(false);
    _backbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didGoToLevelMap = true;
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

std::string GameScene::generateRandomUnitType(std::map<std::string, float> probs) {
    float total = 0.0;
    for (auto const& type : probs) {
        total += type.second;
    }
    float randomChoice = total * ((double) rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const& type : probs) {
        if (i + type.second >= randomChoice) return type.first;
        i += type.second;
    }
    return ""; // Unreachable
}

Unit::Color GameScene::generateRandomUnitColor(std::map<Unit::Color, float> probs) {
    float total = 0.0;
    for (auto const& type : probs) {
        total += type.second;
    }
    float randomChoice = total * ((double) rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const& type : probs) {
        if (i + type.second >= randomChoice) return type.first;
        i += type.second;
    }
    return Unit::Color(0); // Unreachable
}

Vec2 GameScene::generateRandomDirection() {
    vector<cugl::Vec2> allDirs = Unit::getAllPossibleDirections();
    return allDirs[rand() % allDirs.size()];
}

/**
 * sets the correct square texture for a unit.
 * @param square         the square being set
 * @param textures       the texture map being used
 */
void GameScene::updateSquareTexture(shared_ptr<Square> square)
{
    Vec2 squarePos = square->getPosition();
    shared_ptr<Unit> currentUnit = square->getUnit();
    if (_currentReplacementDepth[_board->flattenPos(square->getPosition().x, square->getPosition().y)] + 1 >= _level->maxTurns) { return; }
    shared_ptr<Unit> replacementUnit = _level->getBoard(_currentReplacementDepth[_board->flattenPos(square->getPosition().x, square->getPosition().y)] + 1)->getSquare(square->getPosition())->getUnit();
    std::string color = Unit::colorToString(replacementUnit->getColor());
    string currentDirection = Unit::directionToString(currentUnit->getDirection());
    string replacementDirection = Unit::directionToString(replacementUnit->getDirection());
    string sqTexture;
    if (currentUnit->isSpecial() && (replacementUnit->getSubType() == "basic" || replacementUnit->getSubType() == "random"))
    {
        sqTexture = "special_" + currentDirection + "_square";
    }
    else if (currentUnit->isSpecial() && replacementUnit->getSubType() != "basic" && replacementUnit->getSubType() != "random")
    {
        sqTexture = "arrow-" + currentDirection;
    }
    else if (!currentUnit->isSpecial() && replacementUnit->getSubType() != "basic" && replacementUnit->getSubType() != "random")
    {
        sqTexture = "square-" + replacementDirection + "-" + color;
        // sqTexture = "next-special-square";
    }
    else
    {
        sqTexture = "square";
    }
    square->getViewNode()->setTexture(_textures.at(sqTexture));
}

void GameScene::refreshUnitAndSquareView(shared_ptr<Square> sq) {
    auto unit = sq->getUnit();
    auto unitNode = unit->getViewNode();
    std::string unitSubtype = unit->getSubType();
    unitNode->setTexture(_textures[getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()))]);
    if (unitSubtype != "basic") unit->setSpecial(true);
    else unit->setSpecial(false);
    if (_debug) unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
    updateSquareTexture(sq);
}
/**
 * Generate a unit on the given square.
 *
 * @param sq    The given square pointer
 * @param unitType  The type of unit to generate
 * @param color  The color of the unit to generate
 * @param dir  The direction of the unit to generate
 */
void GameScene::generateUnit(shared_ptr<Square> sq, std::string unitType, Unit::Color color, Vec2 dir, int numberOfUnitsNeededToKill)
{
    // Set Unit Information
    auto unit = sq->getUnit();
    unit->setDirection(dir);
    unit->setColor(color);
    auto unitSelected = _unitTypes[unitType];
    unit->setSubType(unitSelected->getSubType());
    unit->setBasicAttack(unitSelected->getBasicAttack());
    unit->setSpecialAttack(unitSelected->getSpecialAttack());
    unit->setSpecial(unitType != "basic");
    unit->setUnitsNeededToKill(numberOfUnitsNeededToKill);
//    refreshUnitAndSquareView(sq);
}

std::map<Unit::Color, float> GameScene::generateColorProbabilities() {
    std::map<Unit::Color, float> probs = {
        { Unit::Color(0), 0.0 },
        { Unit::Color(1), 0.0 },
        { Unit::Color(2), 0.0 }
    };
    std::map<Unit::Color, int> colorCounts = {
        { Unit::Color(0), 0 },
        { Unit::Color(1), 0 },
        { Unit::Color(2), 0 }
    };
    int totalUnitCount = 0;
    vector<std::shared_ptr<Square>> allSquares = _board->getAllSquares();
    for (auto sq : allSquares) {
        std::shared_ptr<Unit> unit = sq->getUnit();
        Unit::Color unitColor = unit->getColor();
        colorCounts.at(unitColor) = colorCounts.at(unitColor) + 1;
        totalUnitCount++;
    }
    float cumulativeProb = 0.0;
    for (auto const& color : colorCounts) {
        float inverseProbability = static_cast<float>(totalUnitCount) / color.second;
        colorCounts.at(color.first) = inverseProbability;
        cumulativeProb += inverseProbability;
    }
    for (auto const& color : probs) {
        probs.at(color.first) = colorCounts.at(color.first) / cumulativeProb;
    }
    return probs;
}


/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    if (_active)
    {
        removeAllChildren();
        _restartbutton = nullptr;
        _backbutton = nullptr;
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
void GameScene::update(float timestep)
{
    //if (_audioQueue->getState() == cugl::AudioEngine::State::INACTIVE) {
      //  _audioQueue->play(_assets->get<Sound>("track_1"), false, .3, false);
    //}
//    _audioQueue->setLoop(true);

    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    
    if (_turns == 0 && _didRestart == true) reset(_levelJson);
    if (_didRestart == true && _didPause == true) reset(_levelJson);

    if (_turns == 0)
    {
        // Show results screen
        _resultLayout->setVisible(true);
        _restartbutton->activate();
        _backbutton->activate();
        _score_number->setText(to_string(_score));
        _star1->setTexture(_textures.at("star_empty"));
        _star2->setTexture(_textures.at("star_empty"));
        _star3->setTexture(_textures.at("star_empty"));
        if (_level->getNumberOfStars(_score) >= 1) {
            _star1->setTexture(_textures.at("star_full"));
        }
        if (_level->getNumberOfStars(_score) >= 2) {
            _star2->setTexture(_textures.at("star_full"));
        }
        if (_level->getNumberOfStars(_score) >= 3) {
            _star3->setTexture(_textures.at("star_full"));
        }
        return;
    }
    
    if (_didPause == true) {
        // Show settings screen
        _settingsMenuLayout->setVisible(true);
        _settingsBackBtn->activate();
        _settingsRestartBtn->activate();
        _settingsCloseBtn->activate();
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

    if (_input.isDown())
    {
        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable())
        {
            auto squareOnMouse = _board->getSquare(squarePos);
            if (_currentState == SELECTING_UNIT)
            {
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
                _currentState = SELECTING_SWAP;
                
                std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _level->getBoard(_currentReplacementDepth[_board->flattenPos(_selectedSquare->getPosition().x, _selectedSquare->getPosition().y)] + 1)->getSquare(_selectedSquare->getPosition());
                auto upcomingUnitType = replacementSquare->getUnit()->getSubType();
                auto upcomingUnitColor = Unit::colorToString(replacementSquare->getUnit()->getColor());
                auto upcomingUnitDirection =Unit::directionToString(replacementSquare->getUnit()->getDirection());
                auto upcomingSquarePos = _selectedSquare->getViewNode()->getPosition();
                if (upcomingUnitType != "basic" && upcomingUnitType != "random"){
                    _upcomingUnitNode->setPosition(upcomingSquarePos + Vec2(0, _squareSizeAdjustedForScale));
                    _upcomingUnitNode->setScale((float)_squareSizeAdjustedForScale / (float)_defaultSquareSize);
                    auto upcomingDirectionNode = scene2::PolygonNode::allocWithTexture(_textures.at("special_"+upcomingUnitDirection+"_square"));
                    upcomingDirectionNode->setPosition(Vec2(81+_squareSizeAdjustedForScale/2,81+_squareSizeAdjustedForScale));
                    upcomingDirectionNode->setScale(0.8f);
                    _upcomingUnitNode->addChild(upcomingDirectionNode);
                    auto upcomingTextureNode = scene2::PolygonNode::allocWithTexture(_textures.at(upcomingUnitType + "-" + upcomingUnitColor));
                    upcomingTextureNode->setScale(0.8f);
                    upcomingTextureNode->setPosition(Vec2(81+_squareSizeAdjustedForScale/2,81+_squareSizeAdjustedForScale));
                    _upcomingUnitNode->addChild(upcomingTextureNode);
                    _upcomingUnitNode->setVisible(true);
                }
            }
            else if (_currentState == SELECTING_SWAP && squareOnMouse->getPosition().distance(_selectedSquare->getPosition()) == 1)
            {
                _upcomingUnitNode->setVisible(false);
                _upcomingUnitNode->removeAllChildren();

                _currentState = CONFIRM_SWAP;
                _swappingSquare = squareOnMouse;
                // Rotation and Swapping of Model
                //_selectedSquareOriginalDirection = _selectedSquare->getUnit()->getDirection();
                //_swappingSquareOriginalDirection = _swappingSquare->getUnit()->getDirection();
                // We do this so that we can show the attack preview, without changing the actual positional view of units.
                _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                squareOnMouse->getViewNode()->setTexture(_textures.at("square-swap"));
                _attackedSquares = _board->getAttackedSquares(_swappingSquare->getPosition());
                
                for (shared_ptr<Square> attackedSquare : _attackedSquares)
                {
                    attackedSquare->getViewNode()->setTexture(_textures.at("square-attacked"));
                }
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                _upcomingUnitNode->setVisible(false);
                _upcomingUnitNode->removeAllChildren();

                _currentState = SELECTING_SWAP;
                // If we are de-confirming a swap, we must undo the swap.
                _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                //_selectedSquare->getUnit()->setDirection(_selectedSquareOriginalDirection);
                //_swappingSquare->getUnit()->setDirection(_swappingSquareOriginalDirection);
                for (shared_ptr<Square> square : _board->getAllSquares())
                {
                    updateSquareTexture(square);
                }
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
            }
        }
        
    }
    else if (_input.didRelease())
    {
        _upcomingUnitNode->setVisible(false);
        _upcomingUnitNode->removeAllChildren();

        for (shared_ptr<Square> square : _board->getAllSquares())
        {
            updateSquareTexture(square);
        }
        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable() && _currentState == CONFIRM_SWAP)
        {
            std::map<Unit::Color, float> colorProbabilities = generateColorProbabilities();
            //  Because the units in the model where already swapped.
            auto swappedUnitNode = _selectedSquare->getUnit()->getViewNode();
            auto selectedUnitNode = _swappingSquare->getUnit()->getViewNode();
            // Updating View
            _selectedSquare->getViewNode()->removeChild(selectedUnitNode);
            _swappingSquare->getViewNode()->removeChild(swappedUnitNode);
            _selectedSquare->getViewNode()->addChild(swappedUnitNode);
            _swappingSquare->getViewNode()->addChild(selectedUnitNode);

            _prevScore = _score;
            // remove the attacked squares
            for (shared_ptr<Square> attackedSquare : _attackedSquares)
            {
                auto attacked_unit = attackedSquare->getUnit();
                if (_attackedSquares.size() < attackedSquare->getUnit()->getUnitsNeededToKill()) {
                    continue;
                }
                
//                if (attacked_unit->getSubType()=="king") {
//                    std::string unitsNeededToKill = strtool::format("%d/%d",_attackedSquares.size(),attacked_unit->getUnitsNeededToKill());
//                    _info_text->setText(unitsNeededToKill);
//                }
                
                // Replace Unit
                Vec2 squarePos = attackedSquare->getPosition();
                _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)]++;
                int currentReplacementDepth = _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)];
                std::shared_ptr<Square> replacementSquare = _level->getBoard(currentReplacementDepth)->getSquare(squarePos);
                auto unitSubType = replacementSquare->getUnit()->getSubType();
                auto unitColor = unitSubType == "random" ? generateRandomUnitColor(colorProbabilities) : replacementSquare->getUnit()->getColor();
                auto unitDirection = unitSubType == "random" ? generateRandomDirection() : replacementSquare->getUnit()->getDirection();
                if (unitSubType == "random") unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                std:string unitPattern = getUnitType(unitSubType, unitColor);
                generateUnit(attackedSquare, unitSubType, unitColor, unitDirection, replacementSquare->getUnit()->getUnitsNeededToKill());

                // Set empty squares to be uninteractable.
                attackedSquare->setInteractable(unitSubType != "empty");
                attackedSquare->getViewNode()->setVisible(unitSubType != "empty");
                refreshUnitAndSquareView(attackedSquare);
                if (unitSubType == "king") loadKingUI(replacementSquare->getUnit()->getUnitsNeededToKill(), replacementSquare->getUnit()->getUnitsNeededToKill(), squarePos, replacementSquare->getUnit()->getViewNode());
                _score++;
            }
            _turns--;
//             _prev_score = _score;
//             _score += _attackedUnits;
        }
        _currentState = SELECTING_UNIT;
    }
    // Update the score meter
    if (_score >= _level->oneStarThreshold) _scoreMeterStar1->setTexture(_assets->get<Texture>("star_full"));
    if (_score >= _level->twoStarThreshold) _scoreMeterStar2->setTexture(_assets->get<Texture>("star_full"));
    if (_score >= _level->threeStarThreshold) _scoreMeterStar3->setTexture(_assets->get<Texture>("star_full"));
    if (_score < _level->threeStarThreshold) {
        _scoreMeter->setProgress(static_cast<float>(_score) / _level->threeStarThreshold);
        _layout->remove("score_text");
        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + 12 + (static_cast<float>(_score) / _level->threeStarThreshold) * (_scoreMeter->getWidth() - 14),
                                                                                          _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
    } else {
        _scoreMeter->setProgress(1.0f);
        _layout->remove("score_text");
        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + (_scoreMeter->getWidth() - 2),
                                                                                          _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
    }
    _score_text->setText(strtool::format("%d", _score), true);
//    if ((_prev_score < 9 && _score > 9) || (_prev_score < 99 && _score > 99))
//    {
//        _layout->remove("score_text");
//        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -0.85 * (_topuibackgroundNode->getSize().height)));
//    }
    // Update the remaining turns
    _turn_text->setText(strtool::format("%d/%d", _turns, _level->maxTurns));

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
    batch->setColor(Color4::RED);
    //_info_text->render(batch);
    _guiNode->render(batch);

    if (_debug)
    {
        batch->setColor(Color4::RED);
        std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _level->getBoard(_currentReplacementDepth[_board->flattenPos(_selectedSquare->getPosition().x, _selectedSquare->getPosition().y)] + 1)->getSquare(_selectedSquare->getPosition());
        std::string unitType = _selectedSquare == NULL ? "Unit Type: " : "Unit Type: " + _selectedSquare->getUnit()->getSubType();
        std::string unitColor = _selectedSquare == NULL ? "Unit Color: " : "Unit Color: " + Unit::colorToString(_selectedSquare->getUnit()->getColor());
        std::string replacementType = replacementSquare == NULL ? "" : replacementSquare->getUnit()->getSubType();
        std::string replacementColor = replacementSquare == NULL ? "" : Unit::colorToString(replacementSquare->getUnit()->getColor());
        auto direction = _selectedSquare == NULL ? Vec2::ZERO : _selectedSquare->getUnit()->getDirection();
        auto replacementDirection = replacementSquare == NULL ? Vec2::ZERO : replacementSquare->getUnit()->getDirection();
        std::ostringstream unitDirection;
        std::ostringstream replacementUnitDirection;
        unitDirection << "Unit Direction: (" << int(direction.x) << ", " << int(direction.y) << ")";
        replacementUnitDirection << "(" << int(replacementDirection.x) << ", " << int(replacementDirection.y) << ")";
        batch->drawText(unitType, _turn_text->getFont(), Vec2(50, getSize().height - 100));
        batch->drawText(unitColor, _turn_text->getFont(), Vec2(50, getSize().height - 150));
        batch->drawText(unitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 200));
        std::string spe = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->isSpecial() ? "isSpecial() = true"
                                                                                                 : "isSpecial() = false";
        /*
        batch->drawText(spe, _turn_text->getFont(), Vec2(50, getSize().height - 250));
        batch->drawText(replacementType, _turn_text->getFont(), Vec2(50, getSize().height - 300));
        batch->drawText(replacementColor, _turn_text->getFont(), Vec2(50, getSize().height - 350));
        batch->drawText(replacementUnitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 400));
        */
    }

    batch->end();
}
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset()
{
    reset(_assets->get<JsonValue>("level" + std::to_string(_level->levelID)));
}

/**
 * Resets the status of the game with the board JSON.
 */
void GameScene::reset(shared_ptr<cugl::JsonValue> boardJSON)
{
    //    _endgame_text->setForeground(Color4::CLEAR);
    //    _turns = _boardJson->getInt("total-swap-allowed");
    //    _score = 0;
    removeChild(_guiNode);
    _didGoToLevelMap = false;
    _didPause = false;
    init(_assets);
    setLevel(boardJSON);
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

void GameScene::setLevel(shared_ptr<cugl::JsonValue> levelJSON) {
    _levelJson = levelJSON;
    _level = Level::alloc(_boardWidth, _boardHeight, levelJSON);
    vector<int> vector(_boardWidth * _boardHeight, 0);
    _currentReplacementDepth = vector;
    _score = 0;
    _turns = _level->maxTurns;

    map<Unit::Color, float> startingColorProbabilities = {
        { Unit::Color(0), 0.33 },
        { Unit::Color(1), 0.33 },
        { Unit::Color(2), 0.33 }
    };

    // Create the squares & units and put them in the map
    _board->getViewNode()->removeAllChildren();
    for (int i = 0; i < _boardWidth; i++) {
        for (int j = 0; j < _boardHeight; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = Vec2(i, j);
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale * 1.2) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            squareNode->setScale((float)_squareSizeAdjustedForScale / (float)_defaultSquareSize);
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->addChild(squareNode);
            // Generate unit for this square
            auto unit = _level->getBoard(0)->getSquare(squarePosition)->getUnit();
            auto unitSubType = unit->getSubType();
            auto unitColor = Unit::colorToString(unit->getColor());
            Vec2 unitDirection = unit->getDirection();
            
            if (unitSubType == "random") {
                unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                unitColor = Unit::colorToString(generateRandomUnitColor(startingColorProbabilities));
                unitDirection = generateRandomDirection();
            }
            sq->setInteractable(unitSubType != "empty");
            sq->getViewNode()->setVisible(unitSubType != "empty");
            std::string unitPattern = getUnitType(unitSubType, unitColor);
            auto unitTemplate = _unitTypes.at(unitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            shared_ptr<Unit> newUnit = Unit::alloc(unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection, unitSubType != "basic", unit->getUnitsNeededToKill());
            sq->setUnit(newUnit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitPattern));
            newUnit->setViewNode(unitNode);
            newUnit->setSpecial(unitSubType != "basic");
            if (_debug) unitNode->setAngle(newUnit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
            
            // load the ui for king unit
//            auto unit_node = unit->getViewNode();
            if (unitSubType == "king") {
                loadKingUI(unit->getUnitsNeededToKill(), unit->getUnitsNeededToKill(), squarePosition, unitNode);
            }
        }
    }
    // Create the upcoming special unit indicator
    _upcomingUnitNode = scene2::PolygonNode::allocWithTexture(_textures.at("bubble"));
    _boardNode->addChild(_upcomingUnitNode);
    _upcomingUnitNode->setVisible(false);
    _layout->layout(_guiNode.get());

    // Update UI Elements
    _layout->addAbsolute("scoreMeterStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_level->oneStarThreshold) / _level->threeStarThreshold),
        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
    _layout->addAbsolute("scoreMeterStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_level->twoStarThreshold) / _level->threeStarThreshold),
        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
    _layout->addAbsolute("scoreMeterStar3", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + (_scoreMeter->getSize().width),
        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
}

void GameScene::loadKingUI(int unitsKilled, int goal, Vec2 sq_pos, std::shared_ptr<cugl::scene2::PolygonNode> unitNode) {
    std::string unitsNeededToKill = strtool::format("%d", goal);
    _info_text = scene2::Label::allocWithText(unitsNeededToKill, _assets->get<Font>("pixel32"));
    _info_text->setScale(3);
    _info_text->setColor(Color4::RED);
    _info_text->setPosition(Vec2(unitNode->getPosition().x+Vec2::ONE.x, unitNode->getPosition().y+Vec2::ONE.y));
    unitNode->addChildWithName(_info_text, "info");
//    CULog("text has priority of %f", _info_text->getPriority());
//    CULog("unit has priority of %f", unitNode->getPriority());
}
