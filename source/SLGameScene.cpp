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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT / dimen.height;
    if (assets == nullptr)
    {
        return false;
    }
    else if (!Scene2::init(dimen))
    {
        return false;
    }
    // TODO: JSON THIS AND MAKE IT MORE SCALABLE
    // UNIT ATTACK PATTERNS
    std::vector<cugl::Vec2> basicAttack{cugl::Vec2(1, 0)};
    std::vector<cugl::Vec2> diagonalAttack{Vec2(1, 1), Vec2(1, -1), Vec2(-1, 1), Vec2(-1, -1)};
    std::vector<cugl::Vec2> threeWayAttack{Vec2(1, 1), Vec2(1, 0), Vec2(1, -1)};
    std::vector<cugl::Vec2> twoForwardAttack{Vec2(1, 0), Vec2(2, 0)};

    // Start up the input handler
    _input.init();

    // Initialize Variables
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

    // Get the background image and constant values
    _background = assets->get<Texture>("background");

    //layout
    _layout = scene2::AnchoredLayout::alloc();

    //set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());

    // Initialize state
    _currentState = SELECTING_UNIT;

    // Initialize Board
    _board = Board::alloc(BOARD_SIZE, BOARD_SIZE);

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


    // Create and layout the replacement text
    std::string replaceMsg = "Next:";
    _replace_text = scene2::Label::allocWithText(replaceMsg, assets->get<Font>("pixel32"));
    _layout->addAbsolute("replace_text", cugl::scene2::Layout::Anchor::MIDDLE_LEFT, Vec2(0,0));
    _guiNode->addChildWithName(_replace_text, "replace_text");
    


    // Set the view of the board.
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE));
    //_boardNode->setPosition(getSize() / 2);
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER,Vec2(0,0));
    _boardNode->setTexture(transparent_texture);
    _board->setViewNode(_boardNode);

    _guiNode->addChildWithName(_boardNode, "boardNode");

    // Dummy Replacement List  CHANGE WHEN REPLACEMENT CODE IS DONE!!!

    shared_ptr<Unit> aunit = Unit::alloc(Unit::Color::RED, basicAttack, diagonalAttack, Vec2(0, -1));
    shared_ptr<Unit> bunit = Unit::alloc(Unit::Color::GREEN, basicAttack, diagonalAttack, Vec2(0, +1));
    shared_ptr<Unit> cunit = Unit::alloc(Unit::Color::BLUE, basicAttack, diagonalAttack, Vec2(1, 0));
    shared_ptr<Unit> dunit = Unit::alloc(Unit::Color::RED, basicAttack, diagonalAttack, Vec2(-1, 0));

    _replacementListLength = 3;
    _replacementList = {aunit, bunit, cunit, dunit, bunit, bunit, bunit, bunit, bunit, bunit};
    _replacementBoard = Board::alloc(1, _replacementListLength);

    // Set view of replacement list
    _replacementBoardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE));
    //_replacementBoardNode->setPosition(SQUARE_SIZE, getSize().height / 2);
    _replacementBoardNode->setTexture(transparent_texture);
    _replacementBoard->setViewNode(_replacementBoardNode);
    _layout->addRelative("_replacementBoardNode", cugl::scene2::Layout::Anchor::MIDDLE_LEFT, Vec2(.1, 0));

    _guiNode->addChildWithName(_replacementBoardNode, "_replacementBoardNode");


    // Create the squares & units and put them in the map (replacement board)
    for (int i = 0; i < _replacementListLength; i++)
    {

        auto squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
        auto squarePosition = (Vec2(0, i));
        squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
        _replacementBoard->getSquare(squarePosition)->setViewNode(squareNode);
        // Add square node to board node.
        _replacementBoard->getViewNode()->addChild(squareNode);
        // Grab unit from replacement list
        shared_ptr<Unit> unit = _replacementList.at(_replacementListLength - i - 1);
        // Assign Unit to Square
        _replacementBoard->getSquare(squarePosition)->setUnit(unit);
        std::shared_ptr<cugl::Texture> unitTexture;
        if (unit->getColor() == Unit::RED)
        {
            unitTexture = _redUnitTexture;
        }
        else if (unit->getColor() == Unit::GREEN)
        {
            unitTexture = _greenUnitTexture;
        }
        else if (unit->getColor() == Unit::BLUE)
        {
            unitTexture = _blueUnitTexture;
        }
        auto unitNode = scene2::PolygonNode::allocWithTexture(unitTexture);
        unit->setViewNode(unitNode);
        unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
        squareNode->addChild(unitNode);
    }

    // Create the squares & units and put them in the map
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; ++j)
        {
            auto squareNode = scene2::PolygonNode::allocWithTexture(_squareTexture);
            auto squarePosition = (Vec2(i, j));
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * SQUARE_SIZE) + Vec2::ONE * (SQUARE_SIZE / 2));
            _board->getSquare(squarePosition)->setViewNode(squareNode);
            // Add square node to board node.
            _board->getViewNode()->addChild(squareNode);
            // Generate a unit and assign a random color
            shared_ptr<Unit> unit = Unit::alloc(Unit::Color(j % 3), basicAttack, diagonalAttack, Vec2(0, -1));

            // auto randomNumber = rand() % 100;

            /*
            if (randomNumber <= 70) {
            } else if (randomNumber > 70 && randomNumber <= 80) {
                unit.setSpecialAttack(twoForwardAttack);
            } else if (randomNumber > 80 && randomNumber <= 90) {
                unit.setSpecialAttack(threeWayAttack);
            } else {
                unit.setSpecialAttack(diagonalAttack);
            }

            // determine the direction of the unit
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
            unit.setDirection(unitDirection);

            unit.setColor(Unit::Color(randomNumber%3));
            */

            // Assign Unit to Square
            _board->getSquare(squarePosition)->setUnit(unit);

            std::shared_ptr<cugl::Texture> unitTexture;
            if (unit->getColor() == Unit::RED)
            {
                unitTexture = _redUnitTexture;
                /*
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardRedTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayRedTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalRedTexture;
                } else {
                    unitTexture = _redUnitTexture;
                }
                */
            }
            else if (unit->getColor() == Unit::GREEN)
            {
                unitTexture = _greenUnitTexture;
                /*
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardGreenTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayGreenTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalGreenTexture;
                } else {
                    unitTexture = _greenUnitTexture;
                }
                */
            }
            else if (unit->getColor() == Unit::BLUE)
            {
                unitTexture = _blueUnitTexture;
                /*
                if (unit.getSpecialAttack() == twoForwardAttack) {
                    unitTexture = _twoForwardBlueTexture;
                } else if (unit.getSpecialAttack() == threeWayAttack) {
                    unitTexture = _threeWayBlueTexture;
                } else if (unit.getSpecialAttack() == diagonalAttack) {
                    unitTexture = _diagonalBlueTexture;
                } else {
                    unitTexture = _blueUnitTexture;
                }
                */
            }

            auto unitNode = scene2::PolygonNode::allocWithTexture(unitTexture);
            unit->setViewNode(unitNode);
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
        }
    }
    reset();
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    if (_active)
    {
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
                _currentState = CONFIRM_SWAP;
                _swappingSquare = squareOnMouse;
                // Rotation and Swapping of Model
                // We do this so that we can show the attack preview, without changing the actual positional view of units.
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                squareOnMouse->getViewNode()->setTexture(_swapSquareTexture);
                vector<shared_ptr<Square>> attackedSquares = _board->getAttackedSquares(_swappingSquare->getPosition());
                for each (shared_ptr<Square> attackedSquares in attackedSquares)
                {
                    attackedSquares->getViewNode()->setTexture(_attackedSquareTexture);
                }
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                _currentState = SELECTING_SWAP;
                // If we are de-confirming a swap, we must undo the swap.
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                for each (shared_ptr<Square> squares in _board->getAllSquares())
                {
                    squares->getViewNode()->setTexture(_squareTexture);
                }
                _selectedSquare->getViewNode()->setTexture(_selectedSquareTexture);
            }
        }
        else if (_input.didRelease())
        {
            for each (shared_ptr<Square> squares in _board->getAllSquares())
            {
                squares->getViewNode()->setTexture(_squareTexture);
            }
            if (_currentState == CONFIRM_SWAP)
            {
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
            }
            _currentState = SELECTING_UNIT;
        }
    }
    // Update the score meter
    _score_text->setText(strtool::format("Score %d", _score));
    

    // Update the remaining turns
    _turn_text->setText(strtool::format("Turns %d", _turns));

    //Layout everything
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

    //batch->setColor(Color4::RED);
    //batch->drawText(_turn_text, Vec2(10, getSize().height - _turn_text->getBounds().size.height));
    //batch->drawText(_score_text, Vec2(getSize().width - _score_text->getBounds().size.width - 10, getSize().height - _score_text->getBounds().size.height));
    //batch->drawText(_replace_text, Vec2(70, getSize().height - _replace_text->getBounds().size.height - 240));

    batch->end();
}
