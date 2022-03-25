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
    _assets = assets;

    // Acquire the scene built by the asset loader and resize it the scene
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("menu");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
//    _choice = Choice::NONE;
//    _gamebutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("menu_host"));
//    _editorbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("menu_join"));

    std::string titleMsg = strtool::format("Switch Witch");
    auto titleLabel = scene2::Label::allocWithText(titleMsg, assets->get<Font>("title32"));
    scene->addChild(titleLabel);

    // Program the buttons
//    _gamebutton->addListener([this](const std::string& name, bool down) {
//        if (down) {
//            _choice = Choice::GAME;
//        }
//        });
//    _editorbutton->addListener([this](const std::string& name, bool down) {
//        if (down) {
//            _choice = Choice::EDITOR;
//        }
//        });
    //addChild(scene);
    setActive(false);
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
//        if (value) {
//            _choice = NONE;
//            _gamebutton->activate();
//            _editorbutton->activate();
//        }
//        else {
//            CULog("Menu button desactivated");
//            _gamebutton->deactivate();
//            _editorbutton->deactivate();
//            // If any were pressed, reset them
//            _gamebutton->setDown(false);
//            _editorbutton->setDown(false);
//        }
    }
}

