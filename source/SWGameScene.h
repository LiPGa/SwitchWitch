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
#include <cugl/scene2/actions/CUActionManager.h>
#include <cugl/scene2/actions/CUMoveAction.h>
#include <cugl/scene2/actions/CURotateAction.h>
#include <cugl/scene2/actions/CUAnimateAction.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <time.h>
#include "SWSquare.hpp"
#include "SWUnit.hpp"
#include "SWBoard.hpp"
#include "SWLevel.h"
#include "SWInputController.h"
#include <cugl/audio/CUAudioEngine.h>
#include <cugl/scene2/actions/CUActionManager.h>
#include <cugl/scene2/actions/CUMoveAction.h>
#include <cugl/scene2/actions/CUAnimateAction.h>

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
    
    /** Manager to process the animation actions */
    std::shared_ptr<cugl::scene2::ActionManager> _actions;

    // MODELS (Sort-of)
    /** The movement actions */
    std::shared_ptr<cugl::scene2::MoveBy> _moveup;
    std::shared_ptr<cugl::scene2::MoveBy> _movedn;
    std::shared_ptr<cugl::scene2::MoveBy> _moveright;
    std::shared_ptr<cugl::scene2::MoveBy> _moveleft;

//    /** Whether we are mid animation */
//    bool _occupied;

    // MODELS should be shared pointers or a data structure of shared pointers
    /** The JSON value for the levels */
    std::shared_ptr<cugl::JsonValue> _boardJson;
    std::shared_ptr<cugl::JsonValue> _levelJson;
    
    // current level, corresponds to board's ID.
    int _currLevel;
    // if the player has just enter the level
    bool _enterLevel;
    bool _tutorialActive;
    // if the user is scrolling
    bool _isScrolling;
    // if the user opens the help menu
    bool _isHelpMenuOpen;
    std::string _helpBtnPressed;
    // CONSTANTS
    int _maxBoardWidth;
    int _maxBoardHeight;
    int _defaultSquareSize;
    int _defaultUnitSize;
    int _squareSizeAdjustedForScale;
    float _unitSpritePaddingFactor;
    float _squareScaleFactor;
    float _unitScaleFactor;
    std::string _topUI_scores_color;
    std::string _topUI_maxTurn_color;
    std::string _levelID_color;

    
    // hash map for unit textures
    std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> _textures;
    // hash map for units with different types
    std::unordered_map<std::string, std::shared_ptr<Unit>> _unitTypes;
    // hash map for cumulative unit probabilities
    std::unordered_map<std::string, int> _probability;
    // Mapping between unit subtypes and respawn probabilities
    std::map<std::string, float> _unitRespawnProbabilities;
    
//    std::unordered_map<std::string, std::shared_ptr<Action>> _actions;
    
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

    /** The current amount of time since a reset */
    float _time;

    /** The current number of turns left for the player */
    int _turns;
    /** The maximum number of turns for the current level */
    int _maxturns;
    /** The current level ID*/
    int _levelID;
    /** The current score of the player */
    int _score;
    /** The previous score of the player */
    int _prevScore;
    /** Whether the kings has been attacked */
    bool _kingsAttacked;
    /** Whether the king has been killed */
    bool _kingsKilled;
//     /** The number of colors killed */
//     int _attackedColorNum;
//     /** The number of units killed */
//     int _attackedUnits;

    
    /** The current level being played */
    shared_ptr<Level> _level;
    /** _currentCellLayer[i][j] is the current unit lookup depth at cell [i, j] in the board */
    vector<int> _currentReplacementDepth;

#pragma mark -
#pragma mark Model Variables
    /** The board */
    shared_ptr<Board> _board;
    /** The replacement units,unitNodes list */
    //shared_ptr<Board> _replacementBoard;

    /** Square that is currently being selected by the player */
    shared_ptr<Square> _selectedSquare;
    /** Square that is currently being selected by the player to swap with the selectedSquare */
    shared_ptr<Square> _swappingSquare;

    Vec2 _selectedSquareOriginalDirection;
    Vec2 _swappingSquareOriginalDirection;

#pragma mark -
#pragma mark View Variables
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::scene2::OrderedNode> _orderedBoardChild;
    std::shared_ptr<cugl::scene2::PolygonNode> _boardNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _replacementBoardNode;
    std::shared_ptr<cugl::scene2::SceneNode> _guiNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _backgroundNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _topuibackgroundNode;
    std::shared_ptr<cugl::scene2::SceneNode> _helpMenu;
    std::shared_ptr<scene2::SceneNode> _resultLayout;
    std::shared_ptr<scene2::SceneNode> _tutorialLayout;
    std::shared_ptr<scene2::SceneNode> _tutorialLayout2;
    /** result screen when player fails the game*/
    std::shared_ptr<scene2::SceneNode> _failResultLayout;
    std::shared_ptr<scene2::SceneNode> _settingsLayout;
    std::shared_ptr<scene2::SceneNode> _almanacLayout;
    std::shared_ptr<scene2::SceneNode> _almanac;
    std::shared_ptr<scene2::SceneNode> _settingsMenuLayout;
    std::shared_ptr<cugl::scene2::PolygonNode> _upcomingUnitNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _enlargedUnitNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _shieldNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreExplanation;
    
    /** The credit page */
    std::shared_ptr<cugl::scene2::SceneNode> _creditLayout;
    /** The back button on the credit page */
    std::shared_ptr<cugl::scene2::Button>  _aboutBackBtn;
    
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
    /** The text with the maximum turns in the level*/
    std::shared_ptr<cugl::scene2::Label> _maxTurn_text;
    /** The text with the current level ID*/
    std::shared_ptr<cugl::scene2::Label> _levelID_text;
    /** The text with the current score */
    std::shared_ptr<cugl::scene2::Label> _score_text;
    /** The text with the final score */
//    std::shared_ptr<cugl::scene2::Label> _score_number;
//    std::shared_ptr<cugl::scene2::Label> _fail_score_number;
    /** The text with the level number */
    std::shared_ptr<cugl::scene2::Label> _level_info;
    std::shared_ptr<cugl::scene2::Label> _fail_level_info;
    std::shared_ptr<cugl::scene2::Label> _info_text;
    std::shared_ptr<cugl::scene2::Label> _attack_text;
    /** The score meter with the current score */
    std::shared_ptr<cugl::scene2::ProgressBar> _scoreMeter;
//    /** The images of the stars above the score meter */
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar1;
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar2;
    std::shared_ptr<cugl::scene2::PolygonNode> _scoreMeterStar3;
    /** The image of the star in the top left score bar*/
    std::shared_ptr<cugl::scene2::PolygonNode> _oneStar;
    std::shared_ptr<cugl::scene2::PolygonNode> _twoStar1;
    std::shared_ptr<cugl::scene2::PolygonNode> _twoStar2;
    std::shared_ptr<cugl::scene2::PolygonNode> _threeStar1;
    std::shared_ptr<cugl::scene2::PolygonNode> _threeStar2;
    std::shared_ptr<cugl::scene2::PolygonNode> _threeStar3;
    /** The text of score for each star level */
    std::shared_ptr<cugl::scene2::Label> _oneStar_text;
    std::shared_ptr<cugl::scene2::Label> _twoStar_text;
    std::shared_ptr<cugl::scene2::Label> _threeStar_text;
    
    vector<string> _unit_types;
    
    std::string _background_string;
    
    /** The nodes representing unit attacking pattern illustrations */
    std::shared_ptr<cugl::scene2::PolygonNode> _unitPattern1;
    std::shared_ptr<cugl::scene2::PolygonNode> _unitPattern2;
    std::shared_ptr<cugl::scene2::PolygonNode> _unitPattern3;
    std::shared_ptr<cugl::scene2::PolygonNode> _unitPattern4;

    /** The images of the final stars*/
//    std::shared_ptr<cugl::scene2::PolygonNode> _star1;
//    std::shared_ptr<cugl::scene2::PolygonNode> _star2;
//    std::shared_ptr<cugl::scene2::PolygonNode> _star3;
    
    /** The text above the replacement board */
    std::shared_ptr<cugl::scene2::Label> _replace_text;
    /** The button to restart a game */
    std::shared_ptr<cugl::scene2::Button> _restartbutton;
    std::shared_ptr<cugl::scene2::Button> _failRestartButton;
    std::shared_ptr<cugl::scene2::Button> _settingsRestartBtn;
    /** The button to open the help menu  */
    std::shared_ptr<cugl::scene2::Button> _settingsHelpBtn;
    /** The button to close the help menu */
    std::shared_ptr<cugl::scene2::Button> _helpCloseBtn;
    vector<std::string> _helpButtonNames;
    unordered_map<std::string, int> _helpBtnNameToNum;
    unordered_map<std::string, shared_ptr<cugl::scene2::Button>> _helpTutorialBtns;
//    std::shared_ptr<cugl::scene2::Button> _helpAttackBtn;
    /** The animation node in help menu */
    std::shared_ptr<scene2::SpriteNode> _helpAnimationNode;
    /** The button to display settings menu */
    std::shared_ptr<cugl::scene2::Button> _settingsbutton;
    /** The button to display the explanation for score */
    std::shared_ptr<cugl::scene2::Button> _scoreExplanationButton;
    /** The button to close settings menu */
    std::shared_ptr<cugl::scene2::Button> _settingsCloseBtn;
    /** The page of the tutorial*/
    std::shared_ptr<cugl::scene2::Label> _tutorial_page;
    /** The button to close tutorial*/
    std::shared_ptr<cugl::scene2::Button> _tutorialCloseBtn;
    /** The button to go to left page for tutorial*/
    std::shared_ptr<cugl::scene2::Button> _tutorialLeftBtn;
    /** The button to go to right page for tutorial*/
    std::shared_ptr<cugl::scene2::Button> _tutorialRightBtn;
    std::shared_ptr<cugl::scene2::Button> _almanacCloseBtn;
    std::shared_ptr<cugl::scene2::Slider> _musicSlider;
    std::shared_ptr<cugl::scene2::Slider> _soundSlider;
    std::shared_ptr<cugl::AudioQueue>_audioQueue;
    float _musicVolume = .5;
    float _soundVolume = .5;
    float attacksoundmultiplier(float volume, int chaincount);
    /** The button to go back to level map */
    std::shared_ptr<cugl::scene2::Button> _backbutton;
    std::shared_ptr<cugl::scene2::Button> _failBackButton;
    /** The button to go to the next level  */
    std::shared_ptr<cugl::scene2::Button> _nextbutton;
    std::shared_ptr<cugl::scene2::Button> _settingsBackBtn;
    std::shared_ptr<cugl::scene2::Button> _almanacbutton;
    std::shared_ptr<cugl::scene2::Button> _unit1button;
    std::shared_ptr<cugl::scene2::Button> _unit2button;
    std::shared_ptr<cugl::scene2::Button> _unit3button;
    std::shared_ptr<cugl::scene2::Button> _unit4button;
    std::shared_ptr<cugl::scene2::Button> _unit1button_selected;
    std::shared_ptr<cugl::scene2::Button> _unit2button_selected;
    std::shared_ptr<cugl::scene2::Button> _unit3button_selected;
    std::shared_ptr<cugl::scene2::Button> _unit4button_selected;
    vector<shared_ptr<cugl::scene2::Button>> _unitButtons;
    vector<shared_ptr<cugl::scene2::Button>> _unitButtons_selected;
    vector<shared_ptr<cugl::scene2::PolygonNode>> _unitPatterns;
    
    std::shared_ptr<cugl::TextLayout> _winLoseText;
    std::shared_ptr<Square> _kingSquare;
    vector<shared_ptr<Square>> _attackedSquares;
    vector<shared_ptr<Square>> _protectedSquares;
    std::shared_ptr<Square> _initalAttackSquare;

    /** Whther the player pressed restart button*/
    bool _didRestart = false;
    /** Whther the player pressed settings/pause button */
    bool _didPause= false;
    /** Whther the player pressed exit button*/
    bool _didGoToLevelMap = false;
    /** Whther the player pressed next button*/
    bool _didGoToNextLevel = false;
    /** Whther the player pressed almanac button*/
    bool _didPreview = false;
    
    bool unit1Selected = true;
    bool unit2Selected = false;
    bool unit3Selected = false;
    bool unit4Selected = false;
    
    vector<bool> unitMissing = {false, false, false};
    
    bool _midSwap = false;
    
    Size _dimen;
    
    enum ANIMATION_TYPE
    {
        TUTORIAL
    };
    
    std::unordered_map<ANIMATION_TYPE, int, std::hash<int>> animationFrameCounts = {
        { TUTORIAL, 4 }
    };
    
    // ---------------- animation & tutorial ----------------------
    /** The elapsed time since the sprite frame was incremented */
    float _time_since_last_frame = 0.0f;
    /** The elapsed time since the animation started */
    float _time_since_start_animation = 0.0f;
    /** The amout of time every animation should play for */
    float _time_per_animation = 0.9f;
    /** The amout of time every frame should play for */
    float _time_per_frame = 0.3f;
    
    /** This map stores a texture for every tutorial */
    unordered_map<std::string, shared_ptr<Texture>> _tutorialTextures;
    std::shared_ptr<scene2::SpriteNode> _tutorialNode;
    std::shared_ptr<scene2::SceneNode> _tutorialNode2;
    
    

#pragma mark -
#pragma mark Texture Variables
    // TEXTURES SQUARES
    std::shared_ptr<cugl::Texture> _squareTexture;
    std::shared_ptr<cugl::Texture> _selectedSquareTexture;
    std::shared_ptr<cugl::Texture> _attackedSquareTexture;
    std::shared_ptr<cugl::Texture> _swapSquareTexture;

#pragma mark -
//    std::shared_ptr<cugl::AudioQueue> _audioQueue;
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
    
    /**
     * Set up the top UI, including the top background image and star systems
     *
     * @param assets The (loaded) assets for this game mode
     * @param constants The (loaded) constants for this game mode
     */
    void setTopUI(const std::shared_ptr<cugl::AssetManager> &assets, std::shared_ptr<cugl::JsonValue> &constants);
    
    /**
     * Set up the attacking pattern pop-up
     */
    void viewAttackingPatterns();
    
    /**
     * Adjust the unit icons on attacking pattern almanac according to level parameters.
     */
    void updateAttackingPatterns();
    
    void setGoal(std::shared_ptr<cugl::scene2::PolygonNode> squareNode);
    
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
     * Resets the status of the game with the board JSON.
     */
    void reset(std::shared_ptr<cugl::JsonValue> boardJSON);
    
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
    void setLevel(shared_ptr<cugl::JsonValue> levelJSON);

    /**
     * Get the current level of the board
     */
    int getCurrLevel() { return _currLevel; };
    
    /**
     * Sets the current level of the board
     *
     * @param the JSON representation of the board.
     */
    void setCurrLevel( int level_num ) { _currLevel = level_num; }
    
    /**
     * Sets the enter level of the board to true
     */
    void enterLevel( ) { _enterLevel = true; }
    
    /**
     * set the tutorial node according to current level
     */
    void setTutorial();
    
    /**
     * set the help animation
     */
    void setHelpAnimation();
    
    void helpPressButton();
    
//    Size getDimen( ) { return _dimen; }
    
    /**
     * Returns the current state the game is in.
     *
     * @returns the current state
     */
    const bool goToLevelEditor() { return _input.isEscapeDown(); }
    const bool goToLevelMap() { return _didGoToLevelMap; }
    const bool goToNextLevel() { return _didGoToNextLevel; }
    
    /** Sets the cugl::JsonValue that the gamescene reads the board population data from */
    void setBoardJSON(std::shared_ptr<cugl::JsonValue> v) { _boardJson = v; }

    shared_ptr<Level> getLevel() { return _level; }
    
    /**
     * Swaps the animation items on screen
     */
    void doSwap();

    /**
     * Performs a move action
     *
     * @param action The move action
     */
    void doMove(std::string act_key, shared_ptr<cugl::scene2::PolygonNode> viewNode, const std::shared_ptr<cugl::scene2::MoveBy>& action);
    
    /**
     * Check the direction that the selectedSq is moving to
     *
     * @param the selected square
     * @param the square will be swapped with the selected square
     *
     * @return the direction the selected square is moving to
     */
    string moveDirection(shared_ptr<Square> selectedSq, shared_ptr<Square> swappingSq);
    
private:
    /**
     * Generate a random unit type with the given probabilities
     * @param probs    A mapping between unit type and probability. Probabilities should sum to 1.0
     * @return  The randomly generated unit type as a string
     */
    std::string generateRandomUnitType(std::map<std::string, float> probs);
    
    /**
     * Generate a random color with the given probabilities
     * @param probs  A mapping between Unit::Color and probability. Probabilities should sum to 1.0
     * @return  The randomly generated Unit::Color
     */
    Unit::Color generateRandomUnitColor(std::map<Unit::Color, float> probs);
    
    /**
     * Generate a uniformly-random direction
     * @return The random direction as a Vec2
     */
    Vec2 generateRandomDirection();
    
    /**
     * Generates a mapping between Unit::Color and the probability that the color should be respawned.
     * Scans the current board to tally current color counts, and returns probabilities based on the inverted relative sums.
     * @return The mapping between Unit::Color and probabilities
     */
    std::map<Unit::Color, float> generateColorProbabilities();
    
    
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
    
    void loadKingUI(int unitsKilled, int goal, Vec2 sq_pos, std::shared_ptr<cugl::scene2::PolygonNode> unitNode, std::string background);
    
    void updateModelPostSwap();
    
    /**
     * Generate a unit on the given square.
     *
     * @param sq    The given square pointer
     * @param unitType  The type of unit to generate
     * @param color  The color of the unit to generate
     * @param dir  The direction of the unit to generate
     */
    void generateUnit(shared_ptr<Square> sq, std::string unitType, Unit::Color color, Vec2 dir, int numberOfUnitsNeededToKill);
    
    /**
     * Update the view for a  square and its unit, based on the model for that unit.
     * @param sq  The square model to update the view of
     */
    void refreshUnitAndSquareView(shared_ptr<Square> sq);
    
    void refreshUnitView(shared_ptr<Square> sq);

    
    /**
     * Set the indicator color and facing direction
     *
     * @param currentUnit the current unit that will be replaced
     * @param upcomingUnit the special unit that will replace the current unit
     * @return indicatorTexture The corresponding indicator texture name
     */
    std::string setIndicator(shared_ptr<Unit> currentUnit, shared_ptr<Unit> upcomingUnit);

    /**
     * sets the correct square texture for a unit.
     * @param square         the square being set
     * @param textures       the texture map being used
     */
    void updateSquareTexture(shared_ptr<Square> square);
    
    void replaceUnitOnSquare(shared_ptr<Square> sq);
    
    void showResultText(bool success, std::shared_ptr<cugl::scene2::SceneNode> node);
    
    void respawnAttackedSquares();
    
    void deconfirmSwap();
    
    /**
     * shows the tutorial when the player first enter the level
     * @param node               GUI node
     */
    void showTutorial( std::shared_ptr<cugl::scene2::SceneNode> node );
    
    /**
     * flip the tutorial page
     * @param dir                 direction to flip: left or right
     */
    void flipTutorialPage( std::string dir );
};

#endif /* __SW_GAME_SCENE_H__ */
