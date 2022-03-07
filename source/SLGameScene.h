//
//  SLGameScene.h
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
#ifndef __SL_GAME_SCENE_H__
#define __SL_GAME_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>
#include "SWSquare.hpp"
#include "SWUnit.hpp"
#include "SWBoard.hpp"
#include "SWInputController.h"

/**
 * This class is the primary gameplay constroller.
 */
class GameScene : public cugl::Scene2 {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The controller to manage the ship */
    InputController _input;
    
    // MODELS should be shared pointers or a data structure of shared pointers
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    
#pragma mark State Varibales
    /** Possible states of the level.
     * Name of state is describing what the current state of the game is waiting for.
     * For example, the selecting_unit state means the game is waiting for the player to select a unit.
     */
    enum State
    {
        SELECTING_UNIT,
        SELECTING_SWAP,
        CONFIRM_SWAP,
        ANIMATION
    };
    
    /** The current state of the selection process*/
    State _currentState;
    /** The current number of turns left for the player */
    int _turns;
    /** The current score of the player */
    int _score;

#pragma mark -
#pragma mark Model Variables    
    /** The board */
    shared_ptr<Board> _board;
    /** The replacement units list */
    shared_ptr<Board> _replacementBoard;

    std::vector<shared_ptr<Unit>> _replacementList;
    /** Square that is currently being selected by the player */
    shared_ptr<Square> _selectedSquare;
    /** Square that is currently being selected by the player to swap with the selectedSquare */
    shared_ptr<Square> _swappingSquare;

#pragma mark - 
#pragma mark View Variables
    // VIEW
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::scene2::PolygonNode> _boardNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _replacementBoardNode;
    std::shared_ptr<cugl::scene2::SceneNode> _guiNode;
    //std::shared_ptr<cugl::scene2::SceneNode> _boardNodeS;
    //std::shared_ptr<cugl::scene2::SceneNode> _replacementBoardNodeS;


    int _replacementListLength;
    // VIEW items are going to be individual variables
    // In the future, we will replace this with the scene graph
    /** The backgrounnd image */
    std::shared_ptr<cugl::Texture> _background;
    /** The text with the current remaining turns */
    std::shared_ptr<cugl::scene2::Label> _turn_text;
    /** The text with the current score */
    std::shared_ptr<cugl::scene2::Label> _score_text;
    /** The text above the replacement board */
    std::shared_ptr<cugl::scene2::Label> _replace_text;
#pragma mark -
#pragma mark Texture Variables
    //TEXTURES SQUARES
    std::shared_ptr<cugl::Texture> _squareTexture;
    std::shared_ptr<cugl::Texture> _selectedSquareTexture;
    std::shared_ptr<cugl::Texture> _attackedSquareTexture;
    std::shared_ptr<cugl::Texture> _swapSquareTexture;
    
    //TEXTURES UNITS
    std::shared_ptr<cugl::Texture> _redUnitTexture;
    std::shared_ptr<cugl::Texture> _blueUnitTexture;
    std::shared_ptr<cugl::Texture> _greenUnitTexture;
    
    //TEXTURES SPECIAL UNITS - DIAGONAL
    std::shared_ptr<cugl::Texture> _diagonalBlueTexture;
    std::shared_ptr<cugl::Texture> _diagonalGreenTexture;
    std::shared_ptr<cugl::Texture> _diagonalRedTexture;
    
    //TEXTURES SPECIAL UNITS - TWO-FORWARD
    std::shared_ptr<cugl::Texture> _twoForwardBlueTexture;
    std::shared_ptr<cugl::Texture> _twoForwardGreenTexture;
    std::shared_ptr<cugl::Texture> _twoForwardRedTexture;
    
    //TEXTURES SPECIAL UNITS - THREE-WAY
    std::shared_ptr<cugl::Texture> _threeWayBlueTexture;
    std::shared_ptr<cugl::Texture> _threeWayGreenTexture;
    std::shared_ptr<cugl::Texture> _threeWayRedTexture;

#pragma mark -
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::Scene2() {}
    
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    
    
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    
#pragma mark -
#pragma mark Gameplay Handling
    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep) override;

    /**
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

    /**
     * Resets the status of the game so that we can play again.
     */

private:
    /**
     * Returns the score based on the units that have been attacked.
     *
     * The score = the # of units killed times the # of colors killed times the # of special units killed.
     *
     * @param colorNum     The number of colors killed
     * @param basicUnitsNum    The number of basic units killed
     * @param specialUnitsNum The number of special units killed
     * @return    The score of this attack
     */
    int calculateScore(int colorNum, int basicUnitsNum, int specialUnitsNum);
};

#endif /* __SG_GAME_SCENE_H__ */
