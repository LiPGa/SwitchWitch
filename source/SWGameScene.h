//
//  SWGameScene.h
//  SwitchWitch
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This time we are not going to
//  have any additional model classes
//
//  Based on Ship Lab
//  Author: Walker White
//  Version: 1/20/22
//
#ifndef __SW_GAME_SCENE_H__
#define __SW_GAME_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>
#include "SWSquare.hpp"
#include "SWUnit.hpp"
#include "SWBoard.hpp"
#include "SWInputController.h"
#include <cugl/audio/CUAudioEngine.h>

/**
 * This class is the primary gameplay constroller.
 */
class GameScene : public cugl::Scene2
{
protected:
    bool _debug;

    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The controller to manage the ship */
    InputController _input;

    // MODELS should be shared pointers or a data structure of shared pointers
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    /** The JSON value with all of the board members */
    std::shared_ptr<cugl::JsonValue> _boardMembers;
    /** The JSON value for the levels */
    std::shared_ptr<cugl::JsonValue> _boardJson;
    
    // current level, corresponds to board's ID.
    int _currLevel;
    // CONSTANTS
    int _sceneHeight;
    int _boardWidth;
    int _boardHeight;
    int _defaultSquareSize;
    int _squareSizeAdjustedForScale;
    
    // hash map for unit textures
    std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> _textures;
    // hash map for units with different types
    std::unordered_map<std::string, std::shared_ptr<Unit>> _unitTypes;
    // hash map for unit probabilities
    std::unordered_map<std::string, int> _probability;
    
#pragma mark State Varibales
    /** Possible states of the level.
     * Name of state is describing what the current state of the game is waiting for.
     * For example, the selecting_unit state means the game is waiting for the player to select a unit.
     */
    enum State
    {
        NOTHING,
        SELECTING_UNIT,
        SELECTING_SWAP,
        CONFIRM_SWAP,
        ANIMATION
    };

    /** The current state of the selection process*/
    State _currentState;

    /** The scale which all textures must conform to */
    Size _scale;

    /** The current number of turns left for the player */
    int _turns;
    /** The maximum number of turns  */
    int _max_turns;
    /** The current score of the player */
    int _score;
    /** one-star threshold */
    int _onestar_threshold;
    /** two-star threshold*/
    int _twostar_threshold;
    /** three-star threshold*/
    int _threestar_threshold;
    /** The previous score of the player */
    int _prev_score;


    /** The number of colors killed */
    int _attackedColorNum;
    /** The number of basic units killed */
    int _attackedBasicNum;
    /** The number of special units killed */
    int _attackedSpecialNum;

#pragma mark -
#pragma mark Model Variables
    /** The board */
    shared_ptr<Board> _board;
    /** The replacement units,unitNodes list */
    shared_ptr<Board> _replacementBoard;

    std::vector<std::pair<std::shared_ptr<Unit>, std::shared_ptr<scene2::PolygonNode>>> _replacementList;
    /** Square that is currently being selected by the player */
    shared_ptr<Square> _selectedSquare;
    /** Square that is currently being selected by the player to swap with the selectedSquare */
    shared_ptr<Square> _swappingSquare;

    Vec2 _selectedSquareOriginalDirection;
    Vec2 _swappingSquareOriginalDirection;

#pragma mark -
#pragma mark View Variables
    // VIEW
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::scene2::PolygonNode> _boardNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _replacementBoardNode;
    std::shared_ptr<cugl::scene2::SceneNode> _guiNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _backgroundNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _topuibackgroundNode;
    std::shared_ptr<scene2::SceneNode> _resultLayout;


    // std::shared_ptr<cugl::scene2::SceneNode> _boardNodeS;
    // std::shared_ptr<cugl::scene2::SceneNode> _replacementBoardNodeS;

    int _replacementListLength;
    // VIEW items are going to be individual variables
    // In the future, we will replace this with the scene graph
    /** The backgrounnd image */
    std::shared_ptr<cugl::Texture> _background;
    /** The top UI backgrounnd image */
    std::shared_ptr<cugl::Texture> _topuibackground;
    /** The result menu image */
    std::shared_ptr<cugl::Texture> _resultmenubackground;
    /** The text with the current remaining turns */
    std::shared_ptr<cugl::scene2::Label> _turn_text;
    /** The text with the current score */
    std::shared_ptr<cugl::scene2::Label> _score_text;
    /** The text with the final score */
    std::shared_ptr<cugl::scene2::Label> _score_number;
    /** The score meter with the current score */
    std::shared_ptr<cugl::scene2::ProgressBar> _scoreMeter;
//    /** The images of the stars above the score meter */
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar1;
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar2;
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar3;

    /** The images of the final stars*/
    std::shared_ptr<cugl::scene2::PolygonNode> _star1;
    std::shared_ptr<cugl::scene2::PolygonNode> _star2;
    std::shared_ptr<cugl::scene2::PolygonNode> _star3;
    
    /** The text above the replacement board */
    std::shared_ptr<cugl::scene2::Label> _replace_text;
    /** The button to restart  a game */
    std::shared_ptr<cugl::scene2::Button> _restartbutton;
    
    std::shared_ptr<cugl::TextLayout> _winLoseText;
    vector<shared_ptr<Square>> _attackedSquares;
    

    /** Whther the player pressed restart button*/
    bool didRestart = false;

#pragma mark -
#pragma mark Texture Variables
    // TEXTURES SQUARES
    std::shared_ptr<cugl::Texture> _squareTexture;
    std::shared_ptr<cugl::Texture> _selectedSquareTexture;
    std::shared_ptr<cugl::Texture> _attackedSquareTexture;
    std::shared_ptr<cugl::Texture> _swapSquareTexture;

#pragma mark -
    std::shared_ptr<cugl::AudioQueue> _audioQueue;
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
    bool init(const std::shared_ptr<cugl::AssetManager> &assets);

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
    void render(const std::shared_ptr<cugl::SpriteBatch> &batch) override;

    /**
     * Resets the status of the game so that we can play again.
     */

    void reset() override;
    
    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
    /**
     * Sets the board that will be played using JSON
     *
     * @param the JSON representation of the board.
     */
    void setBoard(shared_ptr<cugl::JsonValue> boardJSON);
    
    /**
     * Returns the current state the game is in.
     *
     * @returns the current state
     */
    bool goToLevelEditor() { return _input.isEscapeDown(); }

private:
    /**
     * Get the pattern for a unit provided its type and color
     * @param type     The sub-type of the unit
     * @param color    The coor of the unit
     * @return    The pattern for the unit (texture)
     */
    std::string getUnitType(std::string type, std::string color);

    /**
     * Get the pattern for a unit provided its type and color
     * @param type     The sub-type of the unit
     * @param color    The coor of the unit
     * @return    The pattern for the unit (texture)
     */
    std::string getUnitType(std::string type, Unit::Color color) {
        if (color == Unit::RED) {
            return getUnitType(type, "red");
        }
        else if (color == Unit::GREEN) {
            return getUnitType(type, "green");
        }
        else {
            return getUnitType(type, "blue");
        }
    }
    
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
    
    /**
     * Generate a unit basic or special with random color and direction on the given square.
     *
     * @param sq    The given square
     * @param squareNode    The given squareNode
     *
     */
    void generateUnit(shared_ptr<Square> sq);

    
    /**
     * check if the given position is safe to hold a special unit
     *
     * Among the 8 squares around a special unit, there can be at most one other special unit
     */
    bool isSafe(cugl::Vec2 pos,cugl::Vec2 specialPosition[]);
    

//    void replaceUnit(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode);
//
//    void replaceUnitNoDelete(shared_ptr<Square> sq, shared_ptr<scene2::PolygonNode> squareNode, int i);
//
//    std::pair<std::shared_ptr<Unit>, std::shared_ptr<scene2::PolygonNode>> generateUnitDontSet();

};

#endif /* __SW_GAME_SCENE_H__ */
