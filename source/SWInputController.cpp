//
//  SWInputController.cpp
//  SwitchWitch
//
//  This class buffers in input from the devices and converts it into its
//  semantic meaning. If your game had an option that allows the player to
//  remap the control keys, you would store this information in this class.
//  That way, the main game scene does not have to keep track of the current
//  key mapping.
//
//  Based on Geometry Lab
//  Author: Walker M. White
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include "SWInputController.h"

using namespace cugl;

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
InputController::InputController() : _active(false),
                                     _currDown(false),
                                     _prevDown(false),
                                     _mouseDown(false),
                                     _fingerDown(false),
                                     _upDown(false),
                                     _downDown(false),
                                     _leftDown(false),
                                     _rightDown(false),
                                     _redDown(false),
                                     _greenDown(false),
                                     _blueDown(false),
                                     _saveDown(false),
                                     _playDown(false),
                                     _debugDown(false),
                                     _mouseKey(0),
                                     _touchKey(1)
{
}

/**
 * Initializes the control to support mouse or touch.
 *
 * This method attaches all of the listeners. It tests which
 * platform we are on (mobile or desktop) to pick the right
 * listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @return true if the initialization was successful
 */
bool InputController::init()
{
    Mouse *mouse = Input::get<Mouse>();
    Touchscreen *touch = Input::get<Touchscreen>();
    if (mouse)
    {
        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _mouseKey = mouse->acquireKey();
        mouse->addPressListener(_mouseKey, [=](const cugl::MouseEvent &event, Uint8 clicks, bool focus)
                                { this->buttonDownCB(event, clicks, focus); });
        mouse->addReleaseListener(_mouseKey, [=](const cugl::MouseEvent &event, Uint8 clicks, bool focus)
                                  { this->buttonUpCB(event, clicks, focus); });
        mouse->addDragListener(_mouseKey, [=](const cugl::MouseEvent &event, const Vec2 previous, bool focus)
                               { this->motionCB(event, previous, focus); });
        _active = true;
    }
    else if (touch)
    {
        touch->addBeginListener(_touchKey, [=](const cugl::TouchEvent &event, bool focus)
                                { this->fingerDownCB(event, focus); });
        touch->addEndListener(_touchKey, [=](const cugl::TouchEvent &event, bool focus)
                              { this->fingerUpCB(event, focus); });
        touch->addMotionListener(_touchKey, [=](const cugl::TouchEvent &event, cugl::Vec2 previous, bool focus)
                                 { this->fingerMovedCB(event, previous, focus); });
    }
    return _active;
}

void InputController::dispose()
{
    if (_active)
    {
#ifdef CU_TOUCH_SCREEN
        Touchscreen *touch = Input::get<Touchscreen>();
        touch->removeBeginListener(_touchKey);
        touch->removeEndListener(_touchKey);
        touch->removeMotionListener(_touchKey);
#else
        Mouse *mouse = Input::get<Mouse>();
        mouse->removePressListener(_mouseKey);
        mouse->removeReleaseListener(_mouseKey);
        mouse->removeDragListener(_mouseKey);
        mouse->setPointerAwareness(Mouse::PointerAwareness::BUTTON);
#endif
        _active = false;
    }
}

/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 */
void InputController::update()
{
    _prevDown = _currDown;
    _prevPos = _currPos;

#ifdef CU_TOUCH_SCREEN
    _currDown = _fingerDown;
    _currPos = _touchPos;
#else
    // DESKTOP CONTROLS
    Keyboard *keys = Input::get<Keyboard>();
    // For Level Editor
    _upDown = keys->keyPressed(KeyCode::ARROW_UP) || keys->keyPressed(KeyCode::W);
    _downDown = keys->keyPressed(KeyCode::ARROW_DOWN) || keys->keyPressed(KeyCode::S);
    _leftDown = keys->keyPressed(KeyCode::ARROW_LEFT) || keys->keyPressed(KeyCode::A);
    _rightDown = keys->keyPressed(KeyCode::ARROW_RIGHT) || keys->keyPressed(KeyCode::D);
    _redDown = keys->keyPressed(KeyCode::R) || keys->keyPressed(KeyCode::NUM_1);
    _greenDown = keys->keyPressed(KeyCode::G) || keys->keyPressed(KeyCode::NUM_2);
    _blueDown = keys->keyPressed(KeyCode::B) || keys->keyPressed(KeyCode::NUM_3);
    _saveDown = keys->keyPressed(KeyCode::S) && keys->keyDown(KeyCode::LEFT_CTRL);
    _playDown = keys->keyPressed(KeyCode::ENTER) || keys->keyPressed(KeyCode::KEYPAD_ENTER);
    _debugDown = keys->keyPressed(KeyCode::D) && keys->keyDown(KeyCode::LEFT_CTRL);
    _resetDown = keys->keyPressed(KeyCode::R);
    _escapeDown = keys->keyPressed(KeyCode::ESCAPE);
    
    _currDown = _mouseDown;
    _currPos = _mousePos;
#endif
}

#pragma mark -
#pragma mark Keyboard Presses
/**
 * Returns the arrow key or WSAD key pressed as a vector 2.
 *
 * @returns the cardinal direction selected from arrow keys or WSAD.
 */
cugl::Vec2 InputController::directionPressed() const
{
    if (_leftDown)
    {
        return Vec2::UNIT_X * -1;
    }
    else if (_rightDown)
    {
        return Vec2::UNIT_X;
    }
    else if (_upDown)
    {
        return Vec2::UNIT_Y;
    }
    else if (_downDown)
    {
        return Vec2::UNIT_Y * -1;
    }
    else
    {
        return Vec2::ZERO;
    }
}

#pragma mark -
#pragma mark Mouse Callbacks
/**
 * Call back to execute when a mouse button is first pressed.
 *
 * This function will record a press only if the left button is pressed.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::buttonDownCB(const cugl::MouseEvent &event, Uint8 clicks, bool focus)
{
    // Only recognize the left mouse button
    if (!_mouseDown && event.buttons.hasLeft())
    {
        _mouseDown = true;
        _mousePos = event.position;
    }
}

/**
 * Call back to execute when a mouse button is first released.
 *
 * This function will record a release for the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::buttonUpCB(const cugl::MouseEvent &event, Uint8 clicks, bool focus)
{
    // Only recognize the left mouse button
    if (_mouseDown && event.buttons.hasLeft())
    {
        _mouseDown = false;
    }
}

/**
 * Call back to execute when the mouse moves.
 *
 * This input controller sets the pointer awareness only to monitor a mouse
 * when it is dragged (moved with button down), not when it is moved. This
 * cuts down on spurious inputs. In addition, this method only pays attention
 * to drags initiated with the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param previous  The previously reported mouse location
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::motionCB(const cugl::MouseEvent &event, const Vec2 previous, bool focus)
{
    if (_mouseDown)
    {
        _mousePos = event.position;
    }
}

#pragma mark Touch Callbacks

void InputController::fingerDownCB(const cugl::TouchEvent &event, bool focus)
{
    if (!_fingerDown)
    {
        _fingerDown = true;
        _activeFingerID = make_shared<TouchID>(event.touch);
        _touchPos = event.position;
    }
}

void InputController::fingerUpCB(const cugl::TouchEvent &event, bool focus)
{
    if (_activeFingerID && event.touch == *_activeFingerID)
    {
        _fingerDown = false;
    }
}

void InputController::fingerMovedCB(const cugl::TouchEvent &event, const cugl::Vec2 prev, bool focus)
{
    if (_activeFingerID && event.touch == *_activeFingerID)
    {
        _touchPos = event.position;
    }
}
