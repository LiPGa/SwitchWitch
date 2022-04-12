//
//  SWGameScene.h
//  SwitchWitch
//
//  This is the primary class file for the level editor of our game.
//
//  Based on Ship Lab
//  Author: Liam Riley
//  Version: 3/12/2022
//
#ifndef __SW_LEVEL_EDITOR_SCENE_H__
#define __SW_LEVEL_EDITOR_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include <unordered_set>
#include "SWSquare.hpp"
#include "SWUnit.hpp"
#include "SWBoard.hpp"
#include "SWLevel.h"
#include "SWInputController.h"

/**
 * This class is the primary gameplay constroller.
 */
class LevelEditorScene : public cugl::Scene2
{
protected:
    enum State {
        NOTHING,
        CHANGING_BOARD,
        CHANGING_INFO
    };

    State _currentState;

    /** The scale which all textures must conform to */
    Size _scale;

    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The controller to manage the ship */
    InputController _input;

    // CONSTANTS
    int _boardWidth;
    int _boardHeight;
    int _squareSizeAdjustedForScale;

    // hash map for unit textures
    std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> _textures;
    // hash map for units with different types
    std::unordered_map<std::string, std::shared_ptr<Unit>> _unitTypes;

#pragma mark Model Variables
    /** Boards */
    shared_ptr<Level> _level;
    shared_ptr<Board> _board;
    shared_ptr<Board> _selectionBoard;

    /** Square that is currently being selected by the player */
    shared_ptr<Square> _selectedSquare;
    shared_ptr<Square> _selectedUnitFromSelectionBoard;

    bool _playPressed;
    int _currentBoardTurn;

#pragma mark -
#pragma mark View Variables
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::scene2::PolygonNode> _boardNode;
    std::shared_ptr<cugl::scene2::SceneNode> _rootNode;
    std::shared_ptr<cugl::scene2::SceneNode> _buttonsNode;
    std::shared_ptr<cugl::scene2::SceneNode> _infoNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _selectionBoardNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _backgroundNode;
    std::shared_ptr<cugl::scene2::TextField> _levelIDText;
    std::shared_ptr<cugl::scene2::TextField> _turnText;
    std::shared_ptr<cugl::scene2::TextField> _oneStarScoreText;
    std::shared_ptr<cugl::scene2::TextField> _twoStarScoreText;
    std::shared_ptr<cugl::scene2::TextField> _threeStarScoreText;
    std::shared_ptr<cugl::scene2::Button> _playButton;
    std::shared_ptr<cugl::scene2::Button> _saveButton;
    std::shared_ptr<cugl::scene2::Button> _nextButton;
    std::shared_ptr<cugl::scene2::Button> _backButton;
    std::shared_ptr<cugl::scene2::Button> _deleteButton;
    std::shared_ptr<cugl::scene2::Button> _infoButton;
    std::shared_ptr<cugl::scene2::Button> _boardButton;
    std::shared_ptr<cugl::scene2::Label> _turnTextLabel;

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
    LevelEditorScene() : cugl::Scene2() {}

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~LevelEditorScene() { dispose(); }

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
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;

    /**
     * Gets the level as JSON
     */
    shared_ptr<JsonValue> getLevelAsJSON() { return _level->convertToJSON(); }

    void setLevel(shared_ptr<Level> level) { _level = level; }

    bool goToGameScene() { return _input.isPlayDown() || _playPressed; }

private:
    /**
     * Get the pattern for a unit provided its type and color
     * @param type     The sub-type of the unit
     * @param color    The coor of the unit
     * @return    The pattern for the unit (texture)
     */
    std::string getUnitType(std::string type, std::string color);

    /**
     * Returns if the string is an integer.
     * 
     * @returns true if string is an integer.
     */
    bool isInteger(const std::string& s);


    /**
     * Saves the JSON value in the level directory
     */
    void saveBoardAsJSON();

    /**
     * Allocates a basic board with the first unit of boardMember.json. 
     */
    shared_ptr<Board> allocBasicBoard();

    void updateBoardNode();

    void showInfo();

    void showBoard();
};

#endif /* __SW_GAME_SCENE_H__ */
#pragma once