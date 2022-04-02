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
void updateSquareTexture(shared_ptr<Square> square, std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> textures, shared_ptr<Board> replacementBoard)
{
    Vec2 squarePos = square->getPosition();
    shared_ptr<Unit> currentUnit = square->getUnit();
    shared_ptr<Unit> replacementUnit = replacementBoard->getSquare(squarePos)->getUnit();
    std::string color = Unit::colorToString(replacementUnit->getColor());
    string currentDirection = Unit::directionToString(currentUnit->getDirection());
    string replacementDirection = Unit::directionToString(replacementUnit->getDirection());
    string sqTexture;
    
    
    if (currentUnit->isSpecial() && replacementUnit->getSubType() == "basic")
    {
        sqTexture = "special_" + currentDirection + "_square";
    }
    else if (currentUnit->isSpecial() && replacementUnit->getSubType() != "basic")
    {
        sqTexture = "arrow-" + currentDirection;
    }
    else if (!currentUnit->isSpecial() && replacementUnit->getSubType() != "basic")
    {
        sqTexture = "square-" + replacementDirection + "-" + color;
        // sqTexture = "next-special-square";
    }
    else
    {
        sqTexture="square";
    }
    square->getViewNode()->setTexture(textures.at(sqTexture));
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets, int level_num)
{
    _debug = false;
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
        return false;

    // GetJSONValuesFromAssets
    _constants = assets->get<JsonValue>("constants");
    _boardMembers = assets->get<JsonValue>("boardMember");
    
    _levelSelected = false;
    
    _level = level_num;
    
    string level_name = "level" + std::to_string(_level);
    if (_boardJson == nullptr) _boardJson = assets->get<JsonValue>(level_name);

    // Initialize Constants
    _sceneHeight = _constants->getInt("scene-height");
    vector<int> boardSize = _constants->get("board-size")->asIntArray();
    _boardWidth = boardSize.at(0);
    _boardHeight = boardSize.at(1);
    _squareSizeAdjustedForScale = _constants->getInt("square-size");
    _defaultSquareSize = _constants->getInt("square-size");

    // Initialize Scene
    dimen *= _sceneHeight / dimen.height;
    if (!Scene2::init(dimen))
        return false;

    

    // Start up the input handler
    _input.init();

    // Initialize Variables
    
    _assets = assets;
    _score = 0;
    _prev_score = 0;
    
    // Seed the random number generator to a new seed
    srand(static_cast<int>(time(NULL)));

    if (!didRestart) {
//        _audioQueue = AudioEngine::get()->getMusicQueue();
//        _audioQueue->play(_assets->get<Sound>("track_1"), false, .3, false);
    }
   
    didRestart = false;
    didGoToLevelMap = false;
    
    // Get Textures
    // Preload all the textures into a hashmap
    vector<string> textureVec = _constants->get("textures")->asStringArray();
    for (string textureName : textureVec)
    {
        _textures.insert({textureName, _assets->get<Texture>(textureName)});
    }
    
    // Get Probablities
    // Preload all the probabilities into a hashmap
    vector<string> probabilityVec = _constants->get("probability-respawn")->asStringArray();
    int probabilitySum = 0;
    for (string probabilityName : probabilityVec)
    {
        int respawnProb = _boardMembers-> get("unit") -> get(probabilityName) -> get("probability-respawn") -> asInt();
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
    _replacementBoard = Board::alloc(_boardHeight, _boardWidth);

    // Create and layout the turn meter
    std::string turnMsg = strtool::format("%d/%d", _turns, _max_turns);
    _turn_text = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->setScale(0.75);
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -1.275 * (_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_turn_text, "turn_text");

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
    _layout->addAbsolute("scoreMeterStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 -_topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_onestar_threshold) / _threestar_threshold),
                                                                                           20 + -0.85 * (_topuibackgroundNode->getSize().height)));
    _layout->addAbsolute("scoreMeterStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 -_topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_twostar_threshold) / _threestar_threshold),
                                                                                           20 + -0.85 * (_topuibackgroundNode->getSize().height)));
    _layout->addAbsolute("scoreMeterStar3", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 -_topuibackgroundNode->getSize().width / 6 + (_scoreMeter->getSize().width),
                                                                                           20 + -0.85 * (_topuibackgroundNode->getSize().height)));
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
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height);
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

    importLevel(_boardJson);

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
    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_exit"));
    _backbutton->deactivate();
    _backbutton->setDown(false);
    _backbutton->addListener([this](const std::string& name, bool down) {
        CULog("pressed");
        if (down) {
            CULog("down");
            didGoToLevelMap = true;
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


void GameScene::refreshUnitAndSquareView(shared_ptr<Square> sq) {
    auto unit = sq->getUnit();
    auto unitNode = unit->getViewNode();
    std::string unitSubtype = unit->getSubType();
    unitNode->setTexture(_textures[getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()))]);
    if (unitSubtype != "basic") unit->setSpecial(true);
    else unit->setSpecial(false);
    if (_debug) unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
    updateSquareTexture(sq, _textures, _replacementBoard);
}
/**
 * Generate a unit on the given square.
 *
 * @param sq    The given square pointer
 * @param unitType  The type of unit to generate
 * @param color  The color of the unit to generate
 * @param dir  The direction of the unit to generate
 */
void GameScene::generateUnit(shared_ptr<Square> sq, std::string unitType, Unit::Color color, Vec2 dir)
{
    // Set Unit Information
    auto unit = sq->getUnit();
    unit->setDirection(dir);
    unit->setColor(color);
    std::string unitSubTypeSelected = unitType;
    auto unitSelected = _unitTypes[unitSubTypeSelected];
    unit->setSubType(unitSelected->getSubType());
    unit->setBasicAttack(unitSelected->getBasicAttack());
    unit->setSpecialAttack(unitSelected->getSpecialAttack());

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

void GameScene::copySquareData(std::shared_ptr<Square> from, std::shared_ptr<Square> to) {
    to->setPosition(from->getPosition());
    std::shared_ptr<Unit> fromUnit = from->getUnit();
    std::shared_ptr<Unit> toUnit = to->getUnit();
    toUnit->setColor(fromUnit->getColor());
    toUnit->setDirection(fromUnit->getDirection());
    toUnit->setSpecial(fromUnit->isSpecial());
    toUnit->setSpecialAttack(fromUnit->getSpecialAttack());
    toUnit->setSubType(fromUnit->getSubType());
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
    //if (_audioQueue->getState() == cugl::AudioEngine::State::INACTIVE) {
      //  _audioQueue->play(_assets->get<Sound>("track_1"), false, .3, false);
    //}
//    _audioQueue->setLoop(true);

    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    string level_name = "level" + std::to_string(_level);
    
    if (_levelSelected) {
        importLevel(_assets->get<JsonValue>(level_name));
    }
    
    if (_turns == 0 && didRestart == true){
        CULog("Reset");
        reset(_boardJson);
    }

    if (_turns == 0)
    {
        _restartbutton->activate();
        _backbutton->activate();
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
                
                std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _replacementBoard->getSquare(_selectedSquare->getPosition());
                auto upcomingUnitType = replacementSquare->getUnit()->getSubType();
                auto upcomingUnitColor = Unit::colorToString(replacementSquare->getUnit()->getColor());
                auto upcomingUnitDirection =Unit::directionToString(replacementSquare->getUnit()->getDirection());
                auto upcomingSquarePos = _selectedSquare->getViewNode()->getPosition();
                if (upcomingUnitType != "basic"){
//                    CULog(("special-"+upcomingUnitDirection+"square").c_str());
                    _upcomingUnitNode->setPosition(upcomingSquarePos + Vec2(0, _squareSizeAdjustedForScale));
                    _upcomingUnitNode->setScale((float)_squareSizeAdjustedForScale / (float)_defaultSquareSize);
                    auto upcomingDirectionNode = scene2::PolygonNode::allocWithTexture(_textures.at("special_"+upcomingUnitDirection+"_square"));
//                    CULog("%f %f", upcomingDirectionNode->getPosition().x, upcomingDirectionNode->getPosition().y)
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

                    if (not attackedUnit->isSpecial())
                        _attackedBasicNum++;
                    else
                        _attackedSpecialNum++;
                }
                _attackedColorNum = (int)attackedColors.size();
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                _upcomingUnitNode->setVisible(false);
                _upcomingUnitNode->removeAllChildren();

                _currentState = SELECTING_SWAP;
                // If we are de-confirming a swap, we must undo the swap.
                _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                _selectedSquare->getUnit()->setDirection(_selectedSquareOriginalDirection);
                _swappingSquare->getUnit()->setDirection(_swappingSquareOriginalDirection);
                for (shared_ptr<Square> squares : _board->getAllSquares())
                {
                    updateSquareTexture(squares, _textures, _replacementBoard);
                }
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
            }
        }
        
    }
    else if (_input.didRelease())
    {
        _upcomingUnitNode->setVisible(false);
        _upcomingUnitNode->removeAllChildren();

        for (shared_ptr<Square> squares : _board->getAllSquares())
        {
            updateSquareTexture(squares, _textures,_replacementBoard);
        }
        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable() && _currentState == CONFIRM_SWAP)
        {
            std::map<Unit::Color, float> colorProbabilities = generateColorProbabilities();
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
                Vec2 squarePos = attackedSquare->getPosition();
                std::shared_ptr<Square> replacementSquare = _replacementBoard->getSquare(squarePos);
                copySquareData(replacementSquare, attackedSquare);
                _currentCellLayer[squarePos.x][squarePos.y]++;
                int currentCellDepth = _currentCellLayer[squarePos.x][squarePos.y];
                int cellIndexInOneDimensionalBoard = _board->flattenPos(squarePos.x, squarePos.y);
                CUAssert(currentCellDepth < _unitsInBoard.size()); // The number of turns should be strictly less than the depth of the board!
                auto unitSubType = _unitsInBoard[currentCellDepth].at(cellIndexInOneDimensionalBoard).at(0);
                if (unitSubType == "empty") {
                    attackedSquare->setInteractable(false);
                    attackedSquare->getViewNode()->setVisible(false);
                }
                else {
                    attackedSquare->setInteractable(true);
                    attackedSquare->getViewNode()->setVisible(true);
                }
                if (unitSubType == "random") {
                    // If lookup is random generate random unit
                    auto randomUnitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                    auto randomDirection = generateRandomDirection();
                    auto randomColor = generateRandomUnitColor(colorProbabilities);
                    generateUnit(replacementSquare, randomUnitSubType, randomColor, randomDirection);
                }
                else {
                    // Else generate specific unit
                    auto unitColor = _unitsInBoard[currentCellDepth].at(cellIndexInOneDimensionalBoard).at(1);
                    auto unitDirection = _unitsDirInBoard[currentCellDepth].at(cellIndexInOneDimensionalBoard);
                std:string unitPattern = getUnitType(unitSubType, unitColor);
                    generateUnit(replacementSquare, unitSubType, Unit::stringToColor(unitColor), unitDirection);
                }
                refreshUnitAndSquareView(attackedSquare);
            }

            _turns--;
            _prev_score = _score;
            _score += calculateScore(_attackedColorNum, _attackedBasicNum, _attackedSpecialNum);
        }
        _currentState = SELECTING_UNIT;
    }
    // Update the score meter
    if (_score >= _onestar_threshold) _scoreMeterStar1->setTexture(_assets->get<Texture>("star_full"));
    if (_score >= _twostar_threshold) _scoreMeterStar2->setTexture(_assets->get<Texture>("star_full"));
    if (_score >= _threestar_threshold) _scoreMeterStar3->setTexture(_assets->get<Texture>("star_full"));
    if (_score < _threestar_threshold) {
        _scoreMeter->setProgress(static_cast<float>(_score) / _threestar_threshold);
        _layout->remove("score_text");
        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + 12 + (static_cast<float>(_score) / _threestar_threshold) * (_scoreMeter->getWidth() - 14),
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
        std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _replacementBoard->getSquare(_selectedSquare->getPosition());
        std::string unitType = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->getSubType();
        std::string unitColor = _selectedSquare == NULL ? "" : Unit::colorToString(_selectedSquare->getUnit()->getColor());
        std::string replacementType = replacementSquare == NULL ? "" : replacementSquare->getUnit()->getSubType();
        std::string replacementColor = replacementSquare == NULL ? "" : Unit::colorToString(replacementSquare->getUnit()->getColor());
        auto direction = _selectedSquare == NULL ? Vec2::ZERO : _selectedSquare->getUnit()->getDirection();
        auto replacementDirection = replacementSquare == NULL ? Vec2::ZERO : replacementSquare->getUnit()->getDirection();
        std::ostringstream unitDirection;
        std::ostringstream replacementUnitDirection;
        unitDirection << "(" << int(direction.x) << ", " << int(direction.y) << ")";
        replacementUnitDirection << "(" << int(replacementDirection.x) << ", " << int(replacementDirection.y) << ")";
        batch->drawText(unitType, _turn_text->getFont(), Vec2(50, getSize().height - 100));
        batch->drawText(unitColor, _turn_text->getFont(), Vec2(50, getSize().height - 150));
        batch->drawText(unitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 200));
        std::string spe = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->isSpecial() ? "isSpecial() = true"
                                                                                                 : "isSpecial() = false";
        batch->drawText(spe, _turn_text->getFont(), Vec2(50, getSize().height - 250));
        batch->drawText(replacementType, _turn_text->getFont(), Vec2(50, getSize().height - 300));
        batch->drawText(replacementColor, _turn_text->getFont(), Vec2(50, getSize().height - 350));
        batch->drawText(replacementUnitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 400));
    }

    batch->end();
}
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset(shared_ptr<cugl::JsonValue> boardJSON)
{
    //    _endgame_text->setForeground(Color4::CLEAR);
    //    _turns = _boardJson->getInt("total-swap-allowed");
    //    _score = 0;
    int curr = _level;
    removeChild(_guiNode);
    didGoToLevelMap = false;
    CULog("current level is %d", _level);
    init(_assets, curr);
    string level_name = "level" + std::to_string(_level);
    importLevel(_assets->get<JsonValue>(level_name));
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

void GameScene::setBoard(shared_ptr<cugl::JsonValue> boardJSON) {
    _unitsDirInBoard.clear();
    _unitsInBoard.clear();
    _currentCellLayer.clear();
    
    // Get the sub-type and color of the unit for every unit in this level
    // Get the direction of the unit for every unit in this level and save in a vector
    _currLevel = boardJSON->getInt("id");
    _turns = boardJSON->getInt("total-swap-allowed");
    _max_turns = boardJSON->getInt("total-swap-allowed");
    // thresholds for the star system
    _onestar_threshold = boardJSON->getInt("one-star-condition");
    _twostar_threshold = boardJSON->getInt("two-star-condition");
    _threestar_threshold = boardJSON->getInt("three-star-condition");

    auto layersInBoardJson = boardJSON->get("board-members")->children();
    int layerIndex = 0;
    for (auto layer : layersInBoardJson)
    {
        vector<Vec2> layerDir;
        _unitsDirInBoard.push_back(layerDir);
        vector<vector<std::string>> layerUnit;
        _unitsInBoard.push_back(layerUnit);
        for (auto child : layer->children())
        {
            auto unitDirArray = child->get("direction")->asFloatArray();
            _unitsDirInBoard[layerIndex].push_back(Vec2(unitDirArray.at(0), unitDirArray.at(1)));
            auto unitColor = child->getString("color");
            auto unitSubType = child->getString("sub-type");
            vector<std::string> info{unitSubType, unitColor};
            _unitsInBoard[layerIndex].push_back(info);
        }
        layerIndex++;
    }
    
    map<Unit::Color, float> startingColorProbabilities = {
        { Unit::Color(0), 0.33 },
        { Unit::Color(1), 0.33 },
        { Unit::Color(2), 0.33 }
    };
    
    // Create the squares & units and put them in the map
    _board->getViewNode()->removeAllChildren();
    for (int i = 0; i < _boardWidth; i++) {
        vector<int> cellDepths;
        for (int j = 0; j < _boardHeight; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = Vec2(i, j);
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale*1.2) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            squareNode->setScale((float)_squareSizeAdjustedForScale / (float)_defaultSquareSize);
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            shared_ptr<Square> replacementSq = _replacementBoard->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->addChild(squareNode);
            // Generate unit for this square
            auto unitSubType = _unitsInBoard[0].at(_board->flattenPos(i, j)).at(0);
            auto unitColor = _unitsInBoard[0].at(_board->flattenPos(i, j)).at(1);
            Vec2 unitDirection = _unitsDirInBoard[0].at(_board->flattenPos(i, j));
            Vec2 replacementUnitDirection = _unitsDirInBoard[1].at(_replacementBoard->flattenPos(i, j));
            if (unitSubType == "random") {
                unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                unitColor = Unit::colorToString(generateRandomUnitColor(startingColorProbabilities));
                unitDirection = generateRandomDirection();
            }
            if (unitSubType == "empty") {
                sq->setInteractable(false);
                sq->getViewNode()->setVisible(false);
            }
            else {
                sq->setInteractable(true);
                sq->getViewNode()->setVisible(true);
            }
            auto replacementUnitSubType = _unitsInBoard[1].at(_replacementBoard->flattenPos(i, j)).at(0);
            auto replacementUnitColor = _unitsInBoard[1].at(_replacementBoard->flattenPos(i, j)).at(1);
            if (replacementUnitSubType == "random") {
                replacementUnitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                replacementUnitColor = Unit::colorToString(generateRandomUnitColor(startingColorProbabilities));
                replacementUnitDirection = generateRandomDirection();
            }
            if (replacementUnitSubType == "empty") {
                sq->setInteractable(false);
                sq->getViewNode()->setVisible(false);
            }
            else {
                sq->setInteractable(true);
                sq->getViewNode()->setVisible(true);
            }
            std::string unitPattern = getUnitType(unitSubType, unitColor);
            std::string replacementUnitPattern = getUnitType(replacementUnitSubType, replacementUnitColor);
            auto unitTemplate = _unitTypes.at(unitSubType);
            auto replacementUnitTemplate = _unitTypes.at(replacementUnitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            Unit::Color replacementC = Unit::stringToColor(replacementUnitColor);
            shared_ptr<Unit> unit = Unit::alloc(unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection);
            shared_ptr<Unit> replacementUnit = Unit::alloc(replacementUnitSubType, replacementC, replacementUnitTemplate->getBasicAttack(), replacementUnitTemplate->getSpecialAttack(), replacementUnitDirection);
            sq->setUnit(unit);
            replacementSq->setUnit(replacementUnit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitPattern));
            unit->setViewNode(unitNode);
            if (unitSubType != "basic") unit->setSpecial(true);
            else unit->setSpecial(false);
            if (replacementUnitSubType != "basic") replacementUnit->setSpecial(true);
            else replacementUnit->setSpecial(false);
            if (_debug) unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
            // Initalize _currentCellLayer to vector of zeroes (all cells start with their first-depth unit)
            cellDepths.push_back(0);
        }
        _currentCellLayer.push_back(cellDepths);
    }
    // Create the upcoming special unit indicator
    _upcomingUnitNode = scene2::PolygonNode::allocWithTexture(_textures.at("bubble"));
    _boardNode->addChild(_upcomingUnitNode);
    _upcomingUnitNode->setVisible(false);
    _layout->layout(_guiNode.get());
}

void GameScene::importLevel(shared_ptr<cugl::JsonValue> levelJSON) {
    setBoard(levelJSON);
    _levelSelected = false;
}



