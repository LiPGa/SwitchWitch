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
#include "SLShip.h"
#include "SLPhotonSet.h"
#include "SLAsteroidSet.h"
#include "SWInputController.h"



/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
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
    /** Location and animation information for the ship */
//    std::shared_ptr<Ship> _ship;
    /** The location of all of the active asteroids */
//    AsteroidSet _asteroids;
    
    /** The active units on the board*/
    vector<Unit> _unit_vec;
    /** The active (poly2) units on the board*/
    vector<Poly2> _units;
    /** The active (poly2) squares on the board*/
    vector<Poly2> _squares;
    
    /** The pair with the first = squarePoly and second = unitPoly correspondingly*/
    typedef std::pair<Poly2, Poly2> values;
    
    /** Self-defined hash function for square */
    struct square_hash
    {
        std::size_t operator () (Square const& s) const
        {
            cugl::Vec2 vec2 = s.getPosition();
            size_t rowHash = std::hash<int>()(vec2.x);
            size_t colHash = std::hash<int>()(vec2.y) << 1;
            return rowHash ^ colHash;
        }
    };
    /** A map with key = Square, and value = pair<sqaurePoly, unitPoly> correspondingly */
    std::unordered_map<Square, values, square_hash> _map;
    
    
    /** The board*/
    Board _board;
    
    int _turns;
    int _score;
    
    // VIEW items are going to be individual variables
    // In the future, we will replace this with the scene graph
    /** The backgrounnd image */
    std::shared_ptr<cugl::Texture> _background;
    /** The text with the current remaining turns */
    std::shared_ptr<cugl::TextLayout> _turn_text;
    /** The text with the current score */
    std::shared_ptr<cugl::TextLayout> _score_text;

    
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
    
    void buildScene();
    
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
//    void reset() override;
};

#endif /* __SG_GAME_SCENE_H__ */
