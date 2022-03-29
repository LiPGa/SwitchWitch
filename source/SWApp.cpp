//
//  SWApp.cpp
//  SwitchWitch
//
//  This is the root class for your game.  The file main.cpp accesses this class
//  to run the application.  While you could put most of your game logic in
//  this class, we prefer to break the game up into player modes and have a
//  class for each mode.
//
//  Based on Geometry Lab
//  Author: Walker White
//  Version: 1/20/22
//
#include "SWApp.h"

using namespace cugl;

#pragma mark -
#pragma mark Gameplay Control

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void SwitchWitchApp::onStartup() {
    _assets = AssetManager::alloc();
    _batch  = SpriteBatch::alloc();
    auto cam = OrthographicCamera::alloc(getDisplaySize());
    
#ifdef CU_TOUCH_SCREEN
    // Start-up basic input for loading screen (MOBILE ONLY)
    Input::activate<Touchscreen>();
#else
    // Start-up basic input for loading screen (DESKTOP ONLY)
    Input::activate<Mouse>();
    Input::activate<Keyboard>();
#endif
    
    // Bare bones asset loading (textures only)
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Sound>(SoundLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook()); // Needed for loading screen

    // Create a "loading" screen
    _scene = LOAD;
    _loading.init(_assets);
    
    // Queue up the other assets (EMPTY in this case)
    _assets->loadDirectoryAsync("json/assets.json",nullptr);
    
    AudioEngine::start();
    Application::onStartup(); // YOU MUST END with call to parent
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void SwitchWitchApp::onShutdown() {
    _loading.dispose();
    _gameplay.dispose();
    _levelEditor.dispose();
    _mainMenu.dispose();
    _assets = nullptr;
    _batch = nullptr;

    // Shutdown input
    Input::deactivate<Keyboard>();
    Input::deactivate<Mouse>();

    AudioEngine::stop();
    Application::onShutdown();  // YOU MUST END with call to parent
}

/**
 * The method called when the application is suspended and put in the background.
 *
 * When this method is called, you should store any state that you do not
 * want to be lost.  There is no guarantee that an application will return
 * from the background; it may be terminated instead.
 *
 * If you are using audio, it is critical that you pause it on suspension.
 * Otherwise, the audio thread may persist while the application is in
 * the background.
 */
void SwitchWitchApp::onSuspend() {
    AudioEngine::get()->pause();
}

/**
 * The method called when the application resumes and put in the foreground.
 *
 * If you saved any state before going into the background, now is the time
 * to restore it. This guarantees that the application looks the same as
 * when it was suspended.
 *
 * If you are using audio, you should use this method to resume any audio
 * paused before app suspension.
 */
void SwitchWitchApp::onResume() {
    AudioEngine::get()->resume();
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void SwitchWitchApp::update(float timestep) {
    CULog("current scene is %u", _scene);
    switch (_scene) {
    case LOAD:
        if (_loading.isActive()) {
            _loading.update(timestep);
        }
        else {
            _loading.dispose(); // Permanently disables the input listeners in this mode
            _mainMenu.init(_assets);
            _levelMap.init(_assets);
            _gameplay.init(_assets);
            _levelEditor.init(_assets);
            _mainMenu.setActive(true);
            _scene = State::MENU;
        }
        break;
    case MENU:
        _mainMenu.update(timestep);
        switch (_mainMenu.getChoice()) {
        case MainMenuScene::Choice::MAP:
            _mainMenu.setActive(false);
//            _gameplay.setActive(true);
//            _gameplay.setBoard(_assets->get<JsonValue>("board"));
            _levelMap.setActive(true);
            _scene = State::MAP;
            break;
        case MainMenuScene::Choice::EDITOR:
            _mainMenu.setActive(false);
            _levelEditor.setActive(true);
            _scene = State::EDITOR;
            break;
        case MainMenuScene::Choice::NONE:
            // DO NOTHING
            break;
        }
        break;
    case MAP:
            _levelMap.update(timestep);
            if (_levelMap.getStartStatus()) {
                //_gameplay.setBoard(_assets->get<JsonValue>("board"));
                _levelMap.setActive(false);
                _gameplay.setActive(true);
                _scene = State::GAME;
            }
            break;
    case GAME:
        CULog("%s", "inside game");
        _gameplay.update(timestep);
        if (_gameplay.goToLevelEditor()) {
            CULog("%s", "inside level editor");
            _scene = State::EDITOR;
            _gameplay.setActive(false);
            _levelEditor.setActive(true);
        }
        break;
    case EDITOR:
        _levelEditor.update(timestep);
        if (_levelEditor.goToGameScene()) {
            _scene = State::GAME;
            _gameplay.setBoard(_levelEditor.getBoardAsJSON());
            _levelEditor.setActive(false);
            _gameplay.setActive(true);
        }
    }
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void SwitchWitchApp::draw() {
    switch (_scene) {
    case LOAD:
        _loading.render(_batch);
        break;
    case MAP:
        _levelMap.render(_batch);
        break;
    case MENU:
        _mainMenu.render(_batch);
        break;
    case GAME:
        _gameplay.render(_batch);
        break;
    case EDITOR:
        _levelEditor.render(_batch);
        break;
    }
}


