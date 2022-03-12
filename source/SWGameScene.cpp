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

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** How big the unit radius should be */
#define UNIT_SIZE 50

/** How big the square width should be */
#define SQUARE_SIZE 128

/** How big the board is*/
#define BOARD_SIZE 5

#pragma mark Asset Constants
/** The key for the square texture in the assets manager*/
#define SQUARE_TEXTURE "square"
#define TRANSPARENT_TEXTURE "transparent"
#define RED_UNIT "red-unit"
#define GREEN_UNIT "green-unit"
#define BLUE_UNIT "blue-unit"
#define SQUARE_SELECTED_TEXTURE "square-selected"
#define SQUARE_ATTACKED_TEXTURE "square-attacked"
#define SQUARE_SWAP_TEXTURE "square-swap"
#define DIAGONAL_BLUE "diagonal-blue"
#define DIAGONAL_GREEN "diagonal-green"
#define DIAGONAL_RED "diagonal-red"
#define THREE_WAY_BLUE "three-way-blue"
#define THREE_WAY_GREEN "three-way-green"
#define THREE_WAY_RED "three-way-red"
#define TWO_FORWARD_BLUE "two-forward-blue"
#define TWO_FORWARD_GREEN "two-forward-green"
#define TWO_FORWARD_RED "two-forward-red"

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
    {
        return false;
    }
    
    // Initialize game constants
    _constants = assets->get<JsonValue>("constants");
    _units = assets->get<JsonValue>("units");
    _boardJson = assets->get<JsonValue>("board");
    _sceneHeight = _constants->getInt("scene-height");
    _boardSize = _constants->getInt("board-size");
    _squareSize = _constants->getInt("square-size");
    
    dimen *= _sceneHeight/dimen.height;
    
    if (!Scene2::init(dimen)) {
        return false;
    }

    // TODO: JSON THIS AND MAKE IT MORE SCALABLE
    // UNIT ATTACK PATTERNS

    // Start up the input handler
    _input.init();

    // Initialize Variables
    hasLost = false;
    _assets = assets;
//    _turns = 5;
    _score = 0;
    _prev_score = 0;
    
    // Get Textures
    // preload all the textures into a hashmap
    vector<string> textureVec = _constants->get("textures")->asStringArray();
    for (string textureName : textureVec) {
        _textures.insert({textureName, _assets->get<Texture>(textureName)});
    }
    _squareTexture = _textures.at(SQUARE_TEXTURE);
    _selectedSquareTexture = _textures.at(SQUARE_SELECTED_TEXTURE);
    _swapSquareTexture = _textures.at(SQUARE_SWAP_TEXTURE);
    _attackedSquareTexture = _textures.at(SQUARE_ATTACKED_TEXTURE);
    auto transparent_texture = _textures.at(TRANSPARENT_TEXTURE);

    _redUnitTexture = _textures.at(RED_UNIT);
    _blueUnitTexture = _textures.at(BLUE_UNIT);
    _greenUnitTexture = _textures.at(GREEN_UNIT);

    _diagonalRedTexture = _textures.at(DIAGONAL_RED);
    _diagonalBlueTexture = _textures.at(DIAGONAL_BLUE);
    _diagonalGreenTexture = _textures.at(DIAGONAL_GREEN);

    _twoForwardRedTexture = _textures.at(TWO_FORWARD_RED);
    _twoForwardBlueTexture = _textures.at(TWO_FORWARD_BLUE);
    _twoForwardGreenTexture = _textures.at(TWO_FORWARD_GREEN);

    _threeWayRedTexture = _textures.at(THREE_WAY_RED);
    _threeWayBlueTexture = _textures.at(THREE_WAY_BLUE);
    _threeWayGreenTexture = _textures.at(THREE_WAY_GREEN);

    // Get the background image and constant values
    _background = assets->get<Texture>("background");

    // layout
    _layout = scene2::AnchoredLayout::alloc();

    // set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());

    // Initialize state
    _currentState = SELECTING_UNIT;

    // Initialize Board
    _board = Board::alloc(BOARD_SIZE, BOARD_SIZE);

    // buildScene
    _currLevel = _boardJson->getInt("id");
    _turns = _boardJson->getInt("total-swap-allowed");
    _scoreNeeded = _boardJson->getInt("win-condition");
    // get the type of the unit for every unit in this level
    auto unitsInBoard = _boardJson->get("units")->asStringArray();
    // get the direction of the unit for every unit in this level and save in a vector
    auto unitsDirInBoardJson = _boardJson->get("unit-directions")->children();
    vector<Vec2> unitsDirInBoard;
    for (auto child : unitsDirInBoardJson) {
        auto unitDirArray = child->asFloatArray();
        unitsDirInBoard.push_back(Vec2(unitDirArray.at(0), unitDirArray.at(1)));
    }
    
    // Create and layout the turn meter
    std::string turnMsg = strtool::format("Turns %d", _turns);
    _turn_text = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_LEFT, Vec2(0, -(_turn_text->getTextBounds().size.height)));
    _guiNode->addChildWithName(_turn_text, "turn_text");

    // Create and layout the score meter

    std::string scoreMsg = strtool::format("Score %d", _score);
    _score_text = scene2::Label::allocWithText(scoreMsg, assets->get<Font>("pixel32"));
    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_RIGHT, Vec2(-(_score_text->getTextBounds().size.width), -(_score_text->getTextBounds().size.height)));
    _guiNode->addChildWithName(_score_text, "score_text");

    
    // Set the view of the board.
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE));
    //_boardNode->setPosition(getSize() / 2);
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, 0));
    _boardNode->setTexture(transparent_texture);
    _board->setViewNode(_boardNode);
    
    _guiNode->addChildWithName(_boardNode, "boardNode");
    
    
    // initialize units with different types
    auto children = _units->get("unit-type")->children();
    for (auto child : children) {
        // get color
        string color = child->getString("color");
        Unit::Color c;
        if (color == "red") {
            c = Unit::Color::RED;
        } else if (color == "blue") {
            c = Unit::Color::BLUE;
        } else {
            c = Unit::Color::GREEN;
        }
        
        // get basic attack
        auto basicAttack = child->get("basic-attack")->asFloatArray();
        auto basicAttackVec = vector<Vec2>{Vec2(basicAttack.at(0), basicAttack.at(1))};
        
        // get special attack for this unit
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson) {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        shared_ptr<Unit> unit = Unit::alloc(c, basicAttackVec, specialAttackVec, Vec2(0,-1));
        _unitTypes.insert({child->getString("texture"), unit});
    }
    
    // Create the squares & units and put them in the map
    for (int i=0;i<_boardSize;i++) {
        for(int j=0;j<_boardSize;++j){
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
            auto squarePosition = (Vec2(i,j));
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE/2));
//
//
//    _replacementListLength = 5;
//    for (int i = 0; i < _replacementListLength; i++) {
//        _replacementList.push_back(generateUnitDontSet());
//    }
//    _replacementBoard = Board::alloc(1, _replacementListLength);
//
//    // Set view of replacement list
//    _replacementBoardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE));
//    //_replacementBoardNode->setPosition(SQUARE_SIZE, getSize().height / 2);
//    _replacementBoardNode->setTexture(transparent_texture);
//    _replacementBoard->setViewNode(_replacementBoardNode);
//    _layout->addRelative("_replacementBoardNode", cugl::scene2::Layout::Anchor::MIDDLE_LEFT, Vec2(.1, 0));
//
//    _guiNode->addChildWithName(_replacementBoardNode, "_replacementBoardNode");
//
//    // Create and layout the replacement text
//    std::string replaceMsg = "Next:";
//    _replace_text = scene2::Label::allocWithText(replaceMsg, assets->get<Font>("pixel32"));
//    _layout->addAbsolute("replace_text", cugl::scene2::Layout::Anchor::BOTTOM_LEFT, Vec2(_replacementBoardNode->getPositionX(),0));
//
//    _guiNode->addChildWithName(_replace_text, "replace_text");
//
//    // Create the squares & units and put them in the map (replacement board)
//    for (int i = 0; i < _replacementListLength; i++) {
//            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
//            auto squarePosition = (Vec2(0, i));
//            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
//            shared_ptr<Square> sq = _replacementBoard->getSquare(squarePosition);
//            sq->setViewNode(squareNode);
//            // Add square node to board node.
//            _replacementBoard->getViewNode()->addChild(squareNode);
//            replaceUnitNoDelete(sq, squareNode,i);
//
//    }
//
//    // Create the squares & units and put them in the map
//     // Create the squares & units and put them in the map
//    for (int i = 0; i < BOARD_SIZE; i++) {
//        for (int j = 0; j < BOARD_SIZE; ++j) {
//            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
//            auto squarePosition = (Vec2(i, j));
//            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
            
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            // Add square node to board node.
            _board->getViewNode()->addChild(squareNode);
            // generate unit for this square
            std:string unitType = unitsInBoard.at(_boardSize*(_boardSize-j-1)+i);
            Vec2 unitDirection = unitsDirInBoard.at(_boardSize*(_boardSize-j-1)+i);
            auto unitTemplate = _unitTypes.at(unitType);
            shared_ptr<Unit> unit = Unit::alloc(unitTemplate->getColor(), unitTemplate->getBasicAttack(),unitTemplate->getSpecialAttack(), unitDirection);
            sq->setUnit(unit);
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitType));
            unit->setViewNode(unitNode);
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
        }
    }
    // Create and layout the win lose text
    std::string endgameMsg = "YOU LOSE";
    _endgame_text = scene2::Label::allocWithText(endgameMsg, assets->get<Font>("pixel32"));
    _endgame_text->setForeground(Color4::CLEAR);
    _layout->addAbsolute("endgame_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_endgame_text->getWidth() / 2, -_endgame_text->getHeight()));
    _guiNode->addChildWithName(_endgame_text, "endgame_text");
    
    reset();
    return true;
}

/**
 * Check if the given position is safe to hold a special unit
 *
 * Among the 8 squares around a special unit, there can be at most one other special unit
 */
bool GameScene::isSafe(cugl::Vec2 pos,cugl::Vec2 specialPosition[]) {
    int count = 0;
    for (int i = 0; i < specialPosition->length(); i++) {
        // if the given position is already taken by another special unit
        if (pos == specialPosition[i]) {
            return false;
        }
        
        // if there is any special unit among the 8 adjacent squares of the given position
        if (specialPosition[i].x >= pos.x - 1
            && specialPosition[i].x <= pos.x + 1
            && specialPosition[i].y >= pos.y - 1
            && specialPosition[i].y <= pos.y + 1
            ) {
            count++;
        }
    }
    if (count <= 1) return true;
    return false;
}

/**
 * Generate a unit with random color, attack, and direction on the given square.
 *
 * @param sq    The given square
 * @param squareNode    The given squareNode
 *
 */
void GameScene::generateUnit(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode) {
    // TODO: JSON THIS AND MAKE IT MORE SCALABLE
    std::vector<cugl::Vec2> basicAttack{cugl::Vec2(1,0)};
    std::vector<cugl::Vec2> diagonalAttack{Vec2(1,1), Vec2(1,-1), Vec2(-1,1), Vec2(-1,-1)};
    std::vector<cugl::Vec2> threeWayAttack{Vec2(1,1), Vec2(1,0), Vec2(1,-1)};
    std::vector<cugl::Vec2> twoForwardAttack{Vec2(1,0), Vec2(2,0)};
    
    shared_ptr<Unit> unit = Unit::alloc(Unit::Color(0), basicAttack, {}, Vec2(0,-1));
     
    auto randomNumber = rand() % 100;
    
//    /*
//     * determine the attack pattern of the unit
//     *
//     * number of special units (#special) = 0.1 * total number of units
//     * number of two arrow special = 0.4 * (#special)
//     * number of three arrow special = 0.4 * (#special)
//     * number of four arrow special = 0.2 * (#special)
//     */
//    if (randomNumber <= 90) {
//    } else if (randomNumber > 90 && randomNumber <= 94) {
//        unit->setSpecialAttack(twoForwardAttack);
//    } else if (randomNumber > 94 && randomNumber <= 98) {
//        unit->setSpecialAttack(threeWayAttack);
//    } else {
//        unit->setSpecialAttack(diagonalAttack);
//    }
    
    //determine the direction of the unit
    auto randomNumber2 = rand() % 4;
    Vec2 unitDirection;
    switch (randomNumber2) {
        case 0:
            unitDirection = Vec2(1,0);
            break;
        case 1:
            unitDirection = Vec2(0,1);
            break;
        case 2:
            unitDirection = Vec2(-1,0);
            break;
        default:
            unitDirection = Vec2(0,-1);
            break;
    }
    unit->setDirection(unitDirection);
    
    unit->setColor(Unit::Color(randomNumber%3));
    
    std::shared_ptr<cugl::Texture> unitTexture;
    if (unit->getColor() == Unit::RED) {
        unitTexture = _redUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack) {
//            unitTexture = _twoForwardRedTexture;
//        } else if (unit->getSpecialAttack() == threeWayAttack) {
//            unitTexture = _threeWayRedTexture;
//        } else if (unit->getSpecialAttack() == diagonalAttack) {
//            unitTexture = _diagonalRedTexture;
//        } else {
//            unitTexture = _redUnitTexture;
//        }
    } else if (unit->getColor() == Unit::GREEN) {
        unitTexture = _greenUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack) {
//            unitTexture = _twoForwardGreenTexture;
//        } else if (unit->getSpecialAttack() == threeWayAttack) {
//            unitTexture = _threeWayGreenTexture;
//        } else if (unit->getSpecialAttack() == diagonalAttack) {
//            unitTexture = _diagonalGreenTexture;
//        } else {
//            unitTexture = _greenUnitTexture;
//        }
    } else if (unit->getColor() == Unit::BLUE) {
        unitTexture = _blueUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack) {
//            unitTexture = _twoForwardBlueTexture;
//        } else if (unit->getSpecialAttack() == threeWayAttack) {
//            unitTexture = _threeWayBlueTexture;
//        } else if (unit->getSpecialAttack() == diagonalAttack) {
//            unitTexture = _diagonalBlueTexture;
//        } else {
//            unitTexture = _blueUnitTexture;
//        }
    }
    
    sq->setUnit(unit);
    auto unitNode = scene2::PolygonNode::allocWithTexture(unitTexture);
    unit->setViewNode(unitNode);
    unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
    squareNode->addChild(unitNode);
}

/**
 * Upgrade a basic unit to a special unit.
 *
 * @param sq    The given square
 */
void GameScene::upgradeToSpecial(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode) {
    std::vector<cugl::Vec2> diagonalAttack{Vec2(1,1), Vec2(1,-1), Vec2(-1,1), Vec2(-1,-1)};
    std::vector<cugl::Vec2> threeWayAttack{Vec2(1,1), Vec2(1,0), Vec2(1,-1)};
    std::vector<cugl::Vec2> twoForwardAttack{Vec2(1,0), Vec2(2,0)};

    auto unit = sq->getUnit();
    auto unitNode = unit->getViewNode();
    auto randomNumber3 = rand() % 10 + 1;
    if (randomNumber3 <= 4) {
        unit->setSpecialAttack(twoForwardAttack);
    } else if (randomNumber3 > 4 && randomNumber3 <= 8) {
           unit->setSpecialAttack(threeWayAttack);
    } else {
           unit->setSpecialAttack(diagonalAttack);
    }

    if (unit->getColor() == Unit::RED) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_twoForwardRedTexture);
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_threeWayRedTexture);
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_diagonalRedTexture);
        }
    } else if (unit->getColor() == Unit::GREEN) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_twoForwardGreenTexture);
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_threeWayGreenTexture);
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_diagonalGreenTexture);
        }
    } else if (unit->getColor() == Unit::BLUE) {
        if (unit->getSpecialAttack() == twoForwardAttack) {
            unitNode -> setTexture(_twoForwardBlueTexture);
        } else if (unit->getSpecialAttack() == threeWayAttack) {
            unitNode -> setTexture(_threeWayBlueTexture);
        } else if (unit->getSpecialAttack() == diagonalAttack) {
            unitNode -> setTexture (_diagonalBlueTexture);
        }
    }
}


//std::pair<std::shared_ptr<Unit>, std::shared_ptr<scene2::PolygonNode>> GameScene::generateUnitDontSet()
//{
//    // TODO: JSON THIS AND MAKE IT MORE SCALABLE
//    std::vector<cugl::Vec2> basicAttack{cugl::Vec2(1, 0)};
//    std::vector<cugl::Vec2> diagonalAttack{Vec2(1, 1), Vec2(1, -1), Vec2(-1, 1), Vec2(-1, -1)};
//    std::vector<cugl::Vec2> threeWayAttack{Vec2(1, 1), Vec2(1, 0), Vec2(1, -1)};
//    std::vector<cugl::Vec2> twoForwardAttack{Vec2(1, 0), Vec2(2, 0)};
//
//    shared_ptr<Unit> unit = Unit::alloc(Unit::Color(0), basicAttack, {}, Vec2(0, -1));
//
//    auto randomNumber = rand() % 100;
//
//    if (randomNumber <= 70)
//    {
//    }
//    else if (randomNumber > 70 && randomNumber <= 80)
//    {
//        unit->setSpecialAttack(twoForwardAttack);
//    }
//    else if (randomNumber > 80 && randomNumber <= 90)
//    {
//        unit->setSpecialAttack(threeWayAttack);
//    }
//    else
//    {
//        unit->setSpecialAttack(diagonalAttack);
//    }
//
//    // determine the direction of the unit
//    auto randomNumber2 = rand() % 4;
//    Vec2 unitDirection;
//    switch (randomNumber2)
//    {
//    case 0:
//        unitDirection = Vec2(1, 0);
//        break;
//    case 1:
//        unitDirection = Vec2(0, 1);
//        break;
//    case 2:
//        unitDirection = Vec2(-1, 0);
//        break;
//    default:
//        unitDirection = Vec2(0, -1);
//        break;
//    }
//    unit->setDirection(unitDirection);
//
//    unit->setColor(Unit::Color(randomNumber % 3));
//
//    std::shared_ptr<cugl::Texture> unitTexture;
//    if (unit->getColor() == Unit::RED)
//    {
//        unitTexture = _redUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack)
//        {
//            unitTexture = _twoForwardRedTexture;
//        }
//        else if (unit->getSpecialAttack() == threeWayAttack)
//        {
//            unitTexture = _threeWayRedTexture;
//        }
//        else if (unit->getSpecialAttack() == diagonalAttack)
//        {
//            unitTexture = _diagonalRedTexture;
//        }
//        else
//        {
//            unitTexture = _redUnitTexture;
//        }
//    }
//    else if (unit->getColor() == Unit::GREEN)
//    {
//        unitTexture = _greenUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack)
//        {
//            unitTexture = _twoForwardGreenTexture;
//        }
//        else if (unit->getSpecialAttack() == threeWayAttack)
//        {
//            unitTexture = _threeWayGreenTexture;
//        }
//        else if (unit->getSpecialAttack() == diagonalAttack)
//        {
//            unitTexture = _diagonalGreenTexture;
//        }
//        else
//        {
//            unitTexture = _greenUnitTexture;
//        }
//    }
//    else if (unit->getColor() == Unit::BLUE)
//    {
//        unitTexture = _blueUnitTexture;
//        if (unit->getSpecialAttack() == twoForwardAttack)
//        {
//            unitTexture = _twoForwardBlueTexture;
//        }
//        else if (unit->getSpecialAttack() == threeWayAttack)
//        {
//            unitTexture = _threeWayBlueTexture;
//        }
//        else if (unit->getSpecialAttack() == diagonalAttack)
//        {
//            unitTexture = _diagonalBlueTexture;
//        }
//        else
//        {
//            unitTexture = _blueUnitTexture;
//        }
//    }
//
//    //sq->setUnit(unit);
//    auto unitNode = scene2::PolygonNode::allocWithTexture(unitTexture);
//    unit->setViewNode(unitNode);
//    unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
//    std::pair<std::shared_ptr<Unit>, std::shared_ptr<scene2::PolygonNode>> a = { unit, unitNode };
//    //squareNode->addChild(unitNode);
//    return a;
//}
//
//void GameScene::generateUnit(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode) {
//    auto a = generateUnitDontSet();
//    sq->setUnit(a.first);
//    squareNode->addChild(a.second);
//}
//
//void GameScene::replaceUnit(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode) {
//
//    auto a = _replacementList.front();
//    _replacementList.erase(_replacementList.begin());
//    sq->setUnit(a.first);
//    squareNode->addChild(a.second);
//    _replacementList.push_back(generateUnitDontSet());
//}
//
//void GameScene::replaceUnitNoDelete(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode, int i) {
//
//    auto a = _replacementList.at(i);
//    auto runit = a.first;
//    auto rnode= a.second;
//
//    shared_ptr<Unit> unit = Unit::alloc(runit->getColor(), runit->getBasicAttack(), runit->getSpecialAttack(), runit->getDirection());
//    auto unitNode = scene2::PolygonNode::allocWithTexture(rnode->getTexture());
//    unit->setViewNode(unitNode);
//
//    //_replacementList.erase(_replacementList.begin());
//    sq->setUnit(unit);
//    squareNode->addChild(unitNode);
//    //_replacementList.push_back(generateUnitDontSet());
//}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        removeAllChildren();
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
    return colorNum * (basicUnitsNum + specialUnitsNum) * specialUnitsNum;
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
                _selectedSquare->getViewNode()->setTexture(_selectedSquareTexture);
                _currentState = SELECTING_SWAP;
            }
            else if (_currentState == SELECTING_SWAP && squareOnMouse->getPosition().distance(_selectedSquare->getPosition()) == 1)
            {
                CULog("2");
                _attackedColorNum = 0;
                _attackedBasicNum = 0;
                _attackedSpecialNum = 0;

                _currentState = CONFIRM_SWAP;
                _swappingSquare = squareOnMouse;
                // Rotation and Swapping of Model
                // We do this so that we can show the attack preview, without changing the actual positional view of units.
                CULog("2.5");
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                CULog("2.55");
                squareOnMouse->getViewNode()->setTexture(_swapSquareTexture);
                CULog("2.6");
                vector<shared_ptr<Square>> attackedSquares = _board->getAttackedSquares(_swappingSquare->getPosition());
                
//                _attacked_squares = attackedSquares;
//
//                unordered_set<Unit::Color> attackedColors;
                CULog("3");
//
                _attacked_squares = attackedSquares;

                unordered_set<Unit::Color, hash<int>> attackedColors;

                for (shared_ptr<Square> attackedSquares : attackedSquares)
                {
                    attackedSquares->getViewNode()->setTexture(_attackedSquareTexture);

                    auto attackedUnit = attackedSquares->getUnit();
                    attackedColors.insert(attackedUnit->getColor());

                    if (attackedUnit->getSpecialAttack().size() == 0)
                        _attackedBasicNum++;
                    else
                        _attackedSpecialNum++;
                }
                _attackedColorNum = (int)attackedColors.size();
                CULog("%d %d %d", _attackedColorNum, _attackedBasicNum, _attackedSpecialNum);
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                CULog("4");
                _currentState = SELECTING_SWAP;
                // If we are de-confirming a swap, we must undo the swap.
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                for (shared_ptr<Square> squares : _board->getAllSquares())
                {
                    squares->getViewNode()->setTexture(_squareTexture);
                }
                _selectedSquare->getViewNode()->setTexture(_selectedSquareTexture);
            }
        }
        else if (_input.didRelease())
        {
            for (shared_ptr<Square> squares : _board->getAllSquares())
            {
                squares->getViewNode()->setTexture(_squareTexture);
            }
            if (_currentState == CONFIRM_SWAP)
            {
                CULog("5");
                // TODO: ATTACK
                //  Because the units in the model where already swapped.
                auto swappedUnitNode = _selectedSquare->getUnit()->getViewNode();
                auto selectedUnitNode = _swappingSquare->getUnit()->getViewNode();
                // Rotate Units
                swappedUnitNode->setAngle(_selectedSquare->getUnit()->getAngleBetweenDirectionAndDefault());
                selectedUnitNode->setAngle(_swappingSquare->getUnit()->getAngleBetweenDirectionAndDefault());
                // Updating View
                _selectedSquare->getViewNode()->removeChild(selectedUnitNode);
                _swappingSquare->getViewNode()->removeChild(swappedUnitNode);
                _selectedSquare->getViewNode()->addChild(swappedUnitNode);
                _swappingSquare->getViewNode()->addChild(selectedUnitNode);
                CULog("6");
                //remove the attacked squares
                for (shared_ptr<Square> attackedSquare: _attacked_squares) {
                    auto attacked_unit = attackedSquare->getUnit()->getViewNode();
                    attackedSquare->getViewNode()->removeChild(attacked_unit);
                    generateUnit(attackedSquare, attackedSquare->getViewNode());
                    auto randomNumber4 = rand() % 10;
                    if (randomNumber4 <= 1) {
                        upgradeToSpecial(attackedSquare, attackedSquare->getViewNode());
                    }
                }
                
                _turns--;
                if (_turns == 0)
                {
                 hasLost = true;
                }
                _prev_score = _score;
                    
//
//                // remove the attacked squares
//                for (shared_ptr<Square> attackedSquare : _attacked_squares)
//                {
//                    auto attacked_unit = attackedSquare->getUnit()->getViewNode();
//                    attackedSquare->getViewNode()->removeChild(attacked_unit);
//                    replaceUnit(attackedSquare, attackedSquare->getViewNode());
//                }
//                //change replacement board
//                for (int i = 0; i < _replacementListLength; i++) {
//                    auto rsquare = _replacementBoard->getSquare(Vec2(0, i));
//                    auto runit = rsquare->getUnit()->getViewNode();
//                    rsquare->getViewNode()->removeChild(runit);
//                    replaceUnitNoDelete(rsquare, rsquare->getViewNode(),i);
//                    /*
//                    auto a = _replacementList.at(i);
//                    rsquare->setUnit(a.first);
//                    rsquare->getViewNode()->addChild(a.second);
//                    */
//                }
//
//                _turns--;
//
//                if (_turns == 0)
//                {
//                  hasLost = true;
//                }
//                _prev_score = _score;
                _score += calculateScore(_attackedColorNum, _attackedBasicNum, _attackedSpecialNum);
            }
            _currentState = SELECTING_UNIT;
        }
    }
    // Update the score meter
    _score_text->setText(strtool::format("Score %d", _score),true);
    if ((_prev_score < 9 && _score > 9) || (_prev_score < 99 && _score > 99)) {
        _layout->remove("score_text");
        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_RIGHT, Vec2(-(_score_text->getTextBounds().size.width), -(_score_text->getTextBounds().size.height)));
    }

    //dfghgfd
    if (hasLost) {
        _endgame_text->setForeground(Color4::RED);

    }

    // Update the remaining turns
    _turn_text->setText(strtool::format("Turns %d", _turns));

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
    std::shared_ptr<Vec2> commonOffset = make_shared<Vec2>(getSize() / 2);
    // batch->draw(_background,Rect(Vec2::ZERO, getSize()));
    //_boardNode->render(batch);
    //_replacementBoardNode->render(batch);
    _guiNode->render(batch);

    // batch->setColor(Color4::RED);
    // batch->drawText(_turn_text, Vec2(10, getSize().height - _turn_text->getBounds().size.height));
    // batch->drawText(_score_text, Vec2(getSize().width - _score_text->getBounds().size.width - 10, getSize().height - _score_text->getBounds().size.height));
    // batch->drawText(_replace_text, Vec2(70, getSize().height - _replace_text->getBounds().size.height - 240));
    //_boardNode->render(batch);

    batch->setColor(Color4::BLACK);
    // batch->drawText(_turn_text,Vec2(10, getSize().height-_turn_text->getBounds().size.height));
    // batch->drawText(_score_text, Vec2(getSize().width - _score_text->getBounds().size.width - 10, getSize().height-_score_text->getBounds().size.height));


    batch->end();
}
