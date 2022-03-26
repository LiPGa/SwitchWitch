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

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The vector of levels to choose */
    vector<std::shared_ptr<cugl::scene2::Button>> levels;
//    /** The menu button for level editor */
//    std::shared_ptr<cugl::scene2::Button> _editorbutton;
    /** The player menu choice */
    std::shared_ptr<cugl::scene2::Button> _chosenLevel;
    
    
    // VIEW
    std::shared_ptr<cugl::scene2::SceneNode> _guiNode;
    std::shared_ptr<cugl::Texture> _background;
    std::shared_ptr<cugl::scene2::PolygonNode> _backgroundNode;
    
    cugl::Size _scale;
    
    
    

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
    Choice getLevel() const;
    
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
