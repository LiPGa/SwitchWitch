//
//  SLGameScene.cpp
//  Ship Lab
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This class is a reimagining of
//  the first game lab from 3152 in CUGL.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "SLGameScene.h"

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _input.init();
    
    //Initialize Variables
    _assets = assets;
    _turns = 5;
    _score = 0;
    
    // Get Textures
    _squareTexture = _assets->get<Texture>(SQUARE_TEXTURE);
    _selectedSquareTexture = _assets->get<Texture>(SQUARE_SELECTED_TEXTURE);
    _swapSquareTexture = _assets->get<Texture>(SQUARE_SWAP_TEXTURE);
    _attackedSquareTexture = _assets->get<Texture>(SQUARE_ATTACKED_TEXTURE);
    auto transparent_texture = _assets->get<Texture>(TRANSPARENT_TEXTURE);
    
    _redUnitTexture = _assets->get<Texture>(RED_UNIT);
    _blueUnitTexture = _assets->get<Texture>(BLUE_UNIT);
    _greenUnitTexture = _assets->get<Texture>(GREEN_UNIT);
    
    _diagonalRedTexture = _assets->get<Texture>(DIAGONAL_RED);
    _diagonalBlueTexture = _assets->get<Texture>(DIAGONAL_BLUE);
    _diagonalGreenTexture = _assets->get<Texture>(DIAGONAL_GREEN);
    
    _twoForwardRedTexture = _assets->get<Texture>(TWO_FORWARD_RED);
    _twoForwardBlueTexture = _assets->get<Texture>(TWO_FORWARD_BLUE);
    _twoForwardGreenTexture = _assets->get<Texture>(TWO_FORWARD_GREEN);
    
    _threeWayRedTexture = _assets->get<Texture>(THREE_WAY_RED);
    _threeWayBlueTexture = _assets->get<Texture>(THREE_WAY_BLUE);
    _threeWayGreenTexture = _assets->get<Texture>(THREE_WAY_GREEN);
    
    _currentState = SELECTING_UNIT;
    //Initialize Board
    _board = std::shared_ptr<Board>(new Board(BOARD_SIZE, BOARD_SIZE));
    // Ensuring that these values are never null.
    _selectedSquare = _board->getSquare(Vec2::ZERO);
    _swappingSquare = _board->getSquare(Vec2::ZERO);

    // Get the background image and constant values
    _background = assets->get<Texture>("background");
    
    //TODO: buildScene is broken code.
    //buildScene();
    
    // Create and layout the turn meter
    std::string turnMsg = strtool::format("Turns %d", _turns);
    _turn_text = TextLayout::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->layout();
    
    // Create and layout the score meter
    std::string scoreMsg = strtool::format("Score %d", _score);
    _score_text = TextLayout::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _score_text->layout();
    
    // Set the view of the board.
    Rect testRect = Rect(0,0,BOARD_SIZE*SQUARE_SIZE,BOARD_SIZE*SQUARE_SIZE);
    _boardNode = scene2::PolygonNode::allocWithPoly(testRect);
    _boardNode->setPosition(getSize()/2);
    _boardNode->setTexture(transparent_texture);
    
    //TODO: JSON THIS AND MAKE IT MORE SCALABLE
    // UNIT ATTACK PATTERNS
    std::vector<cugl::Vec2> basicAttack{cugl::Vec2(1,0)};
    std::vector<cugl::Vec2> diagonalAttack{Vec2(1,1), Vec2(1,-1), Vec2(-1,1), Vec2(-1,-1)};
    std::vector<cugl::Vec2> threeWayAttack{Vec2(1,1), Vec2(1,0), Vec2(1,-1)};
    std::vector<cugl::Vec2> twoForwardAttack{Vec2(1,0), Vec2(2,0)};
    
    // Create the squares & units and put them in the map
    for (int i=0;i<BOARD_SIZE;i++) {
        for(int j=0;j<BOARD_SIZE;++j){
            auto squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
            auto squarePosition = (Vec2(i,j));
            squareNode->setPosition((squarePosition * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE/2));
            
            // Generate a unit and assign a random color
            Unit unit = Unit();
            unit.setBasicAttack(basicAttack);
            
            auto randomNumber = rand() % 100;
        
            if (randomNumber <= 70) {
                
            } else if (randomNumber > 70 && randomNumber <= 80) {
                unit.setSpecialAttack(twoForwardAttack);
            } else if (randomNumber > 80 && randomNumber <= 90) {
                unit.setSpecialAttack(threeWayAttack);
            } else {
                unit.setSpecialAttack(diagonalAttack);
            }
        
            unit.setColor(Unit::Color(randomNumber%3));
            
            // Assign Unit to Square
            _board->getSquare(squarePosition).setUnit(unit);
            std::shared_ptr<cugl::Texture> unitTexture = _redUnitTexture;
            CULog("Unit Color %d", unit.getColor());
            if (unit.getColor() == unit.RED) {
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardRedTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayRedTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalRedTexture;
                } else {
                    unitTexture = _redUnitTexture;
                }
            } else if (unit.getColor() == unit.GREEN) {
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardGreenTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayGreenTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalGreenTexture;
                } else {
                    unitTexture = _greenUnitTexture;
                }
            } else if (unit.getColor() == unit.BLUE) {
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardBlueTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayBlueTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalBlueTexture;
                } else {
                    unitTexture = _blueUnitTexture;
                }
            }
            
            
            auto unitNode = scene2::PolygonNode::allocWithTexture(unitTexture);
            squareNode->addChild(unitNode);
            
            /** Reason why BOARD_SIZE - 1 - j is because the mouse coordinates are
             * calculated from the top left corner of the screen
             * while the position of nodes are calculated from the bottom left of the node.
             * Therefore, in order to allign with mouse position, the squares are stored with
             * a tag that goes backwards in y position.
             */
            _boardNode->addChildWithTag(squareNode, squarePosToTag(Vec2(i,BOARD_SIZE - 1 - j)));
        }
    }
    reset();
    return true;
}
/**
 * Returns the tag of a square node based on the position of the square.
 *
 * The children of a node are organized by tag number.
 * This method returns the appropriate tag number given the position of a square.
 *
 * @param x     The x position of the square
 * @param y     The y position of the square
 * @return    The tag of the square node
 */
int GameScene::squarePosToTag(const int x, const int y) {
    return x * BOARD_SIZE + y;
}

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
int GameScene::calculateScore(int colorNum, int basicUnitsNum, int specialUnitsNum){
    return colorNum * (basicUnitsNum + specialUnitsNum) * specialUnitsNum;
}

/**
 * Returns the position of a square node based on the tag number of a square node.
 * The children of a node are organized by tag number.
 *
 * @param tagNum The tag number of the node
 * @return The position of the square in a Vec2
 */
cugl::Vec2 GameScene::tagToSquarePos(const int tagNum) {
    int y = tagNum % BOARD_SIZE;
    int x = (tagNum - y) / BOARD_SIZE;
    return Vec2(x, y);
}

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
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {
    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    Vec2 pos = _input.getPosition();
    Vec2 boardPos = _boardNode->worldToNodeCoords(pos);
    Vec2 squarePos = Vec2(int(boardPos.x) / SQUARE_SIZE, int(boardPos.y) / SQUARE_SIZE);
    if (_board->doesSqaureExist(squarePos)){
        auto square = _board->getSquare(squarePos);
        auto squareNode = _boardNode->getChildByTag<cugl::scene2::PolygonNode>(squarePosToTag(squarePos));
        
        if (_input.isDown()) {
            if (_currentState == SELECTING_UNIT) {
                _selectedSquare = square;
                squareNode->setTexture(_selectedSquareTexture);
                _currentState = SELECTING_SWAP;
            }
            else if (_currentState == SELECTING_SWAP) {
                _boardNode->getChildByTag<cugl::scene2::PolygonNode>(squarePosToTag(_swappingSquare.getPosition()))->setTexture(_squareTexture);
                if (square.getPosition() != _selectedSquare.getPosition() && square.getPosition().distance(_selectedSquare.getPosition()) <= 1) {
                    _swappingSquare = square;
                    squareNode->setTexture(_swapSquareTexture);
                }
            }
             
        }
        else if (_input.didRelease()){
            if (_currentState == SELECTING_SWAP){
                _boardNode->getChildByTag<cugl::scene2::PolygonNode>(squarePosToTag(_swappingSquare.getPosition()))->setTexture(_squareTexture);
                _boardNode->getChildByTag<cugl::scene2::PolygonNode>(squarePosToTag(_selectedSquare.getPosition()))->setTexture(_squareTexture);
                if (_selectedSquare.getPosition() != _swappingSquare.getPosition() && squarePos == _swappingSquare.getPosition()){
                    //TODO: ATTACK
                }
                _currentState = SELECTING_UNIT;
            }
        }
         
    }
    
    // Update the score meter
    _score_text->setText(strtool::format("Score %d", _score));
    _score_text->layout();
    
    // Update the remaining turns
    _turn_text->setText(strtool::format("Turns %d", _turns));
    _turn_text->layout();
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
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    std::shared_ptr<Vec2> commonOffset = make_shared<Vec2>(getSize() / 2);
    // batch->draw(_background,Rect(Vec2::ZERO, getSize()));
    _boardNode->render(batch);
    
    batch->setColor(Color4::BLACK);
    batch->drawText(_turn_text,Vec2(10, getSize().height-_turn_text->getBounds().size.height));
    batch->drawText(_score_text, Vec2(getSize().width - _score_text->getBounds().size.width - 10, getSize().height-_score_text->getBounds().size.height));

    batch->end();
}
