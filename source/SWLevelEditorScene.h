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

    // CONSTANTS
    int _sceneHeight;
    int _boardSize;
    int _squareSize;

    // hash map for unit textures
    std::unordered_map<std::string, std::shared_ptr<cugl::Texture>> _textures;
    // hash map for units with different types
    std::unordered_map<std::string, std::shared_ptr<Unit>> _unitTypes;

#pragma mark Model Variables
    /** Boards */
    shared_ptr<Board> _board;
    shared_ptr<Board> _selectionBoard;

    /** Square that is currently being selected by the player */
    shared_ptr<Square> _selectedSquare;
    shared_ptr<Square> _selectedUnitFromSelectionBoard;

    int numberOfTurns;
    int id;
    int winCondition;

#pragma mark -
#pragma mark View Variables
    // VIEW
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::scene2::PolygonNode> _boardNode;
    std::shared_ptr<cugl::scene2::SceneNode> _guiNode;
    std::shared_ptr<cugl::scene2::PolygonNode> _selectionBoardNode;

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
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

private:
    /**
     * Get the pattern for a unit provided its type and color
     * @param type     The sub-type of the unit
     * @param color    The coor of the unit
     * @return    The pattern for the unit (texture)
     */
    std::string getUnitType(std::string type, std::string color);

};

#endif /* __SW_GAME_SCENE_H__ */
#pragma once