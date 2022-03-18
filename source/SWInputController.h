//
//  SWInputController.h
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
#ifndef __SW_INPUT_CONTROLLER_H__
#define __SW_INPUT_CONTROLLER_H__

#include <stdio.h>
#include <cugl/cugl.h>
/**
 * Device-independent input manager.
 *
 * This class supports drag controls. This means pressing at a location,
 * dragging while pressed, and then releasing. However, the implementation
 * for this approach varies according to whether it is a mouse or mobile
 * touch controls. This interface hides the details.
 */

 /** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

class InputController {
// Stylistically, I like common stuff protected, device specific private
protected:
    /** Whether the input device was successfully initialized */
    bool _active;
    /** The current touch/mouse position */
    cugl::Vec2 _currPos;
    /** The previous touch/mouse position */
    cugl::Vec2 _prevPos;
    /** Whether there is an active button/touch press */
    bool _currDown;
    /** Whether there was an active button/touch press last frame*/
    bool _prevDown;

    /** Whether the debug key is down*/
    bool _debugDown;

    // Inputs used for level editor
    /** Whether the up key is down*/
    bool _upDown;
    /** Whether the down key is down*/
    bool _downDown;
    /** Whether the left key is down*/
    bool _leftDown;
    /** Whether the right key is down*/
    bool _rightDown;
    /** Whether the red key is down*/
    bool _redDown;
    /** Whether the green key is down*/
    bool _greenDown;
    /** Whether the blue key is down*/
    bool _blueDown;
    /** Whether the save key is down*/
    bool _saveDown;
    /** Whether the play key is down*/
    bool _playDown;
    /** Whether the restart key is down*/
    bool _resetDown;
    /** Whether the escape key is down*/
    bool _escapeDown;
    
protected:
	/** The key for the mouse listeners */
	Uint32 _mouseKey;
    /** The mouse position (for mice-based interfaces) */
    cugl::Vec2 _mousePos;
    /** Whether the (left) mouse button is down */
    bool _mouseDown;

protected:
    Uint32 _touchKey;
    cugl::Vec2 _touchPos;
    bool _fingerDown;
    shared_ptr<cugl::TouchID> _activeFingerID;
    
#pragma mark Input Control
public:
    /**
     * Creates a new input controller.
     *
     * This constructor DOES NOT attach any listeners, as we are not
     * ready to do so until the scene is created. You should call
     * the {@link #init} method to initialize the scene.
     */
    InputController();

    /**
     * Deletes this input controller, releasing all resources.
     */
    ~InputController() { dispose(); }
    
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
    bool init();
    
    
    /**
     * Disposes this input controller, deactivating all listeners.
     *
     * As the listeners are deactived, the user will not be able to
     * monitor input until the controller is reinitialized with the
     * {@link #init} method.
     */
    void dispose();
    
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
    void update();

#pragma mark Attributes
    /**
     * Returns true if this control is active.
     *
     * An active control is one where all of the listeners are attached
     * and it is actively monitoring input. An input controller is only
     * active if {@link #init} is called, and if {@link #dispose} is not.
     *
     * @return true if this control is active.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the current mouse/touch position
     *
     * @return the current mouse/touch position
     */
    const cugl::Vec2& getPosition() const {
        return _currPos;
    }

    /**
     * Returns the previous mouse/touch position
     *
     * @return the previous mouse/touch position
     */
    const cugl::Vec2& getPrevious() const {
        return _prevPos;
    }
    
    /**
     * Return true if the user initiated a press this frame.
     *
     * A press means that the user is pressing (button/finger) this
     * animation frame, but was not pressing during the last frame.
     *
     * @return true if the user initiated a press this frame.
     */
    bool didPress() const {
        return !_prevDown && _currDown;
    }
    /**
     * Return true if the user initiated a reset in this frame.
     *
     * @return true if the user initiated a press this reset.
     */
    bool didPressReset() const {
        return _resetDown;
    }

    /**
     * Return true if the user initiated a release this frame.
     *
     * A release means that the user was pressing (button/finger) last
     * animation frame, but is not pressing during this frame.
     *
     * @return true if the user initiated a release this frame.
     */
    bool didRelease() const {
        return !_currDown && _prevDown;
    }

    /**
     * Return true if the user is actively pressing this frame.
     *
     * This method only checks that a press is active or ongoing.
     * It does not care when the press was initiated.
     *
     * @return true if the user is actively pressing this frame.
     */
    bool isDown() const {
        return _currDown;
    }

#pragma mark -
#pragma mark Debug and Level Editor Controls
    /**
     * Returns the arrow key or WSAD key pressed as a vector 2.
     * 
     * @returns the cardinal direction selected from arrow keys or WSAD.
     */
    cugl::Vec2 directionPressed() const;

    /**
     * Return true if a direction key is pressed this frame.
     *
     * This method only checks that a press is active or ongoing.
     * It does not care when the press was initiated.
     *
     * @return true if the user is actively pressing a direction key this frame.
     */
    bool isDirectionKeyDown() const {
        return directionPressed() != cugl::Vec2::ZERO;
    }

    /**
     * Returns wether the red button is down. Used in level editor.
     *
     * @returns if the red button is down.
     */
    bool isRedDown() { return _redDown; }

    /**
     * Returns wether the green button is down. Used in level editor.
     *
     * @returns if the green button is down.
     */
    bool isGreenDown() { return _greenDown; }

    /**
     * Returns wether the blue button is down. Used in level editor.
     *
     * @returns if the green button is down.
     */
    bool isBlueDown() { return _blueDown; }

    /**
     * Returns wether the save button is down. Used in level editor.
     * 
     * @returns if the save button is down.
     */
    bool isSaveDown() { return _saveDown; }
    
    /**
     * Returns wether the play button is down. Used in level editor.
     *
     * @returns if the play button is down.
     */
    bool isPlayDown() { return _playDown; }

    /**
     * Returns if the debug key is down.
     *
     * @returns if the debug key is down.
     */
    bool isDebugDown() const {
        return _debugDown;
    }

    /**
     * Returns if the escape key is down.
     *
     * @returns if the escape key is down.
     */
    bool isEscapeDown() const { return _escapeDown; }

#pragma mark -
#pragma mark Mouse Callbacks
private:
    /**
     * Call back to execute when a mouse button is first pressed.
     *
     * This function will record a press only if the left button is pressed.
     *
     * @param event     The event with the mouse information
     * @param clicks    The number of clicks (for double clicking)
     * @param focus     Whether this device has focus (UNUSED)
     */
    void buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

    /**
     * Call back to execute when a mouse button is first released.
     *
     * This function will record a release for the left mouse button.
     *
     * @param event     The event with the mouse information
     * @param clicks    The number of clicks (for double clicking)
     * @param focus     Whether this device has focus (UNUSED)
     */
    void buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

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
    void motionCB(const cugl::MouseEvent& event, const cugl::Vec2 previous, bool focus);
    
#pragma mark Touch Callbacks
private:
    void fingerDownCB(const cugl::TouchEvent& event, bool focus);
    void fingerUpCB(const cugl::TouchEvent& event, bool focus);
    void fingerMovedCB(const cugl::TouchEvent& event, const cugl::Vec2 previous, bool focus);
};




#endif /* __SW_INPUT_CONTROLLER_H__ */
