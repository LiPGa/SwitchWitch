//
//  SWLevelMapScene.cpp
//  SwitchWitch
//
//  Created by Ashley on 3/24/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//



#include "SWLevelMapScene.hpp"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720


#pragma mark -
#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
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
bool LevelMapScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT / dimen.height;
    if (assets == nullptr) {
        return false;
    }
    else if (!Scene2::init(dimen)) {
        return false;
    }

    // Start up the input handler
    _input.init();
    _assets = assets;
    
    // Set up GUI
//    start_ok = false;
    
    _chosenLevel = 0;
    _num_levels = 19;
    loadLevelButtons();

    _scrollPane = std::dynamic_pointer_cast<scene2::ScrollPane>(_assets->get<scene2::SceneNode>("map"));
    _scrollPane->setContentSize(dimen);
    _scrollPane->doLayout();
    
    _currentState = NOACTION;
    
    addChild(_scrollPane);
    
    
//    _layout = scene2::AnchoredLayout::alloc();

    setActive(false);
    _audioQueue = AudioEngine::get()->getMusicQueue();
    _audioQueue->play(_assets->get<Sound>("normal"), true, .3, false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LevelMapScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

void LevelMapScene::loadLevelButtons() {
    for (std::size_t i=1; i <= _num_levels; ++i) {
        string name = "map_background_level" + std::to_string(i);
                auto level = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>(name));
                level->addListener([this, i](const std::string& name, bool down) {
                    if (down) {
                        _chosenLevel = static_cast<int> (i);
                        //start_ok = true;
                    }
                });
        levels.push_back(level);
    }
}


/**
 * Sets whether the scene is currently active
 *
 * This method should be used to toggle all the UI elements.  Buttons
 * should be activated when it is made active and deactivated when
 * it is not.
 *
 * @param value whether the scene is currently active
 */
void LevelMapScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _chosenLevel = NO_LEVEL;
            for (auto level : levels) {
                level->activate();
            }
        }
        else {
            CULog("Menu button desactivated");
            for (auto level : levels) {
                level->deactivate();
                level->setDown(false);
            }
            // If any were pressed, reset them
        }
    }
}

void LevelMapScene::update(float dt) {
    _input.update();

    // Process the toggled key commands
    Vec2 pos = _input.getPosition();
    if (_currentState == SCROLLING) {
        Vec2 prevPos = _input.getPrevious();
        Vec2 moveDist = Vec2(pos.x-prevPos.x, prevPos.y-pos.y);
        Vec2 nodePos = _scrollPane->getPosition();
        moveDist.x = 0;
//        CULog("scroll pane content size: %f", _scrollPane->getContentSize().height);
//        CULog("scroll pane size: %f", _scrollPane->getSize().height);
//        CULog("scroll pane bounding box: %f", _scrollPane->getBoundingBox().size.height);
//        if (moveDist.y + nodePos.y < -_scrollPane->getContentSize().height) {
        if (moveDist.y + nodePos.y < -2792+720) {
            moveDist.y = -2792+720 - nodePos.y;
        } else if (moveDist.y + nodePos.y > 0) {
            moveDist.y = 0 - nodePos.y;
        }
        _scrollPane->setPosition(nodePos+moveDist*0.4);
    }
    if (_input.isDown()) {
        _currentState = SCROLLING;
    } else if (_input.didRelease()) {
        _currentState = NOACTION;
    }
}


void LevelMapScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch) {
    batch->begin(getCamera()->getCombined());
    _scrollPane->render(batch);
    batch->end();
}

