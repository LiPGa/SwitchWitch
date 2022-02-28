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
#define SQUARE_SIZE 120

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
    _assets = assets;
    _board = Board(5, 5);
    _turns = 5;
    _score = 0;

    // Get the background image and constant values
    _background = assets->get<Texture>("background");
    buildScene();
    
    // Create and layout the turn meter
    std::string turnMsg = strtool::format("Turns %d", _turns);
    _turn_text = TextLayout::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->layout();
    
    // Create and layout the score meter
    std::string scoreMsg = strtool::format("Score %d", _score);
    _score_text = TextLayout::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _score_text->layout();
    
    reset();
    return true;
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
    
    
    // Update the score meter
    _score_text->setText(strtool::format("Score %d", _score));
    _score_text->layout();
    
    // Update the remaining turns
    _turn_text->setText(strtool::format("Turns %d", _turns));
    _turn_text->layout();
}

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
    
    std::shared_ptr<PolyFactory> polyFactory = make_shared<PolyFactory>();

    for (int i=0;i<5;i++) {
        for(int j=0;j<5;++j){
            // draw the square
            auto square = _board.getSquare(Vec2(i,j));
            Vec2 squareOrigin = square.getPosition();
            Poly2 squarePoly = polyFactory->makeRect(squareOrigin, Vec2(SQUARE_SIZE, SQUARE_SIZE));
            batch->setColor(Color4::BLACK);
            batch->outline(Rect(squarePoly), *commonOffset);
            batch->setColor(Color4::WHITE);
            batch->fill(squarePoly, *commonOffset);
            
            // draw the unit
            auto unit = square.getUnit();
            auto unitOrigin = squareOrigin + Vec2(SQUARE_SIZE/2, SQUARE_SIZE/2);
            Poly2 unitPoly = polyFactory->makeCircle(unitOrigin, UNIT_SIZE);
            batch->setColor(unit.getColor());
            batch->fill(unitPoly, *commonOffset);
        }
    }
    
    batch->setColor(Color4::BLACK);
    batch->drawText(_turn_text,Vec2(10, getSize().height-_turn_text->getBounds().size.height));
    batch->drawText(_score_text, Vec2(getSize().width - _score_text->getBounds().size.width - 10, getSize().height-_score_text->getBounds().size.height));

    batch->end();
}

void GameScene::buildScene(){
    for(int i=0;i<5;++i){
        for(int j=0;j<5;++j){
            auto squareOrigin = Vec2(SQUARE_SIZE*(i-2)-SQUARE_SIZE/2, SQUARE_SIZE*(j-2)-SQUARE_SIZE/2);
            // Generate a unit and assign a random color
            std::shared_ptr<Unit> unit = make_shared<Unit>();
            unit->setColor(Unit::Colors(rand()%3));
            
            // Link the square to the unit
            std::shared_ptr<Square> square = make_shared<Square>();
            square->setPosition(squareOrigin);
            square->setUnit(*unit);
            
            _board.setSquare(Vec2(i, j), *square);

        }
    }
}
