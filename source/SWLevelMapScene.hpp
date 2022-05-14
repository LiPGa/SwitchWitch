//
//  SWLevelMapScene.hpp
//  SwitchWitch
//
//  Created by Ashley on 3/24/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWLevelMapScene_hpp
#define SWLevelMapScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include <vector>
#include "SWInputController.h"


/**
 * This class presents the menu to the player.
 *
 * There is no need for an input controller, as all input is managed by
 * listeners on the scene graph.  We only need getters so that the main
 * application can retrieve the state and communicate it to other scenes.
 */
class LevelMapScene : public cugl::Scene2 {
public:
    /**
     * The menu choice.
     *
     * This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to play the game */
        GAME,
        /** User wants to use the editor */
        EDITOR
    };
    
#pragma mark State Varibales
    /** Possible states of the level.
     * Name of state is describing what the current state of the game is waiting for.
     * For example, the selecting_unit state means the game is waiting for the player to select a unit.
     */
    enum State
    {
        /** User has not yet made a move */
        NOACTION,
        /** User is scrolling the screen */
        SCROLLING
    };
    
    enum Level
    {
        NO_LEVEL,
        LEVEL_ONE,
        LEVEL_TWO,
        LEVEL_THREE,
        LEVEL_FOUR,
        LEVEL_FIVE,
        LEVEL_SIX,
        LEVEL_SEVEN,
        LEVEL_EIGHT,
        LEVEL_NINE,
        LEVEL_TEN
    };
    
    /** The current state of the selection process*/
    State _currentState;

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The vector of levels to choose */
    int _num_levels;
    vector<std::shared_ptr<cugl::scene2::Button>> levels;
    
    int _chosenLevel;

    bool start_ok = false;
    
    // CONTROLLERS are attached directly to the scene (no pointers)
    InputController _input;
    
    // VIEW
    std::shared_ptr<cugl::scene2::SceneNode> _scrollPane;
    std::shared_ptr<cugl::Texture> _background;
    //std::shared_ptr<cugl::Texture> _levelOneTexture;
    //std::shared_ptr<cugl::scene2::PolygonNode> _backgroundNode;
    //std::shared_ptr<cugl::scene2::SceneNode> _levelOneNode;
    cugl::Size _scale;
    std::shared_ptr<cugl::scene2::AnchoredLayout> _layout;
    std::shared_ptr<cugl::AudioQueue> _audioQueue;
    std::shared_ptr<cugl::audio::AudioMixer> _audioMixer;
    std::shared_ptr<cugl::AudioSample> _normalSample;
    std::shared_ptr<cugl::AudioSample> _iceSample;

    std::shared_ptr<cugl::AudioSample> _fireSample;

    std::shared_ptr<cugl::audio::AudioPlayer> _normalNode;

    std::shared_ptr<cugl::audio::AudioNode> _fireNode;
    std::shared_ptr<cugl::audio::AudioNode> _iceNode;

    
    

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    LevelMapScene() : cugl::Scene2() {}

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~LevelMapScene() { dispose(); }

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;

    /**
     * Initializes the controller contents.
     *
     * In previous labs, this method "started" the scene.  But in this
     * case, we only use to initialize the scene user interface.  We
     * do not activate the user interface yet, as an active user
     * interface will still receive input EVEN WHEN IT IS HIDDEN.
     *
     * That is why we have the method {@link #setActive}.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

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
     * Returns the user's chosen level.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's chosen level.
     */
    int getLevel() const { return _chosenLevel; }
    
    void loadLevelButtons();
    void adjust_scroll_bgm(float yposition);
    void snapbgm(int _chosenLevel);
    
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
};

#endif /* SWLevelMapScene_hpp */
