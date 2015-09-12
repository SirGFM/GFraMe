/**
 * @file src/include/GFraMe_int/gfmInput.h
 * 
 * Generic input interface; Acts as frontend for the gfmEvent backend; It's used
 * by creating virtual keys, which can be bind to multiple physical keys/buttons
 * at once. If a key bound to a virtual key is pressed, this virtual key is 
 * guaranteed to keep a "just pressed" state for a frame (instead of requiring
 * the user to check for that).
 * It also should make customizable constrols easier to implement, since it only
 * requires the binding/mapping to be modified.
 * 
 * Each physical key/button/etc is restricted to a single virtual key. This can
 * be somewhat limiting, but it should be quite easy to work around it.
 * 
 * Sometime in the future, there will be also "virtual axis", in a way to allow
 * it to be mapped to either a controller axis or the mouse position
 * effortlessly. (well, I can dream...)
 */
#ifndef __GFMINPUT_STRUCT__
#define __GFMINPUT_STRUCT__

typedef struct enGFMInput gfmInput;

enum enGFMInputState {
    gfmInput_released     = 0x01,
    gfmInput_pressed      = 0x02,
    gfmInput_justPressed  = 0x06,
    gfmInput_justReleased = 0x09,
    gfmInput_stateMask    = 0x03,
    gfmInput_justMask     = 0x0C,
    gfmInput_curFrame     = 0x0F
};
typedef enum enGFMInputState gfmInputState;

/** Map to input interface; Either a keyboard key, a controller button or a
    mouse button; Note that the controller must be the last device, as a
    controller button is multiplied by its ID */
enum enGFMInputIface {
    gfmIface_none = 0,
    /** Keyboard keys */
    gfmKey_q,
    gfmKey_w,
    gfmKey_e,
    gfmKey_r,
    gfmKey_t,
    gfmKey_y,
    gfmKey_u,
    gfmKey_i,
    gfmKey_o,
    gfmKey_p,
    gfmKey_a,
    gfmKey_s,
    gfmKey_d,
    gfmKey_f,
    gfmKey_g,
    gfmKey_h,
    gfmKey_j,
    gfmKey_k,
    gfmKey_l,
    gfmKey_z,
    gfmKey_x,
    gfmKey_c,
    gfmKey_v,
    gfmKey_b,
    gfmKey_n,
    gfmKey_m,
    gfmKey_f1,
    gfmKey_f2,
    gfmKey_f3,
    gfmKey_f4,
    gfmKey_f5,
    gfmKey_f6,
    gfmKey_f7,
    gfmKey_f8,
    gfmKey_f9,
    gfmKey_f10,
    gfmKey_f11,
    gfmKey_f12,
    gfmKey_1,
    gfmKey_2,
    gfmKey_3,
    gfmKey_4,
    gfmKey_5,
    gfmKey_6,
    gfmKey_7,
    gfmKey_8,
    gfmKey_9,
    gfmKey_0,
    gfmKey_n1,
    gfmKey_n2,
    gfmKey_n3,
    gfmKey_n4,
    gfmKey_n5,
    gfmKey_n6,
    gfmKey_n7,
    gfmKey_n8,
    gfmKey_n9,
    gfmKey_n0,
    gfmKey_nReturn,
    gfmKey_lctrl,
    gfmKey_lalt,
    gfmKey_lshift,
    gfmKey_rctrl,
    gfmKey_ralt,
    gfmKey_rshift,
    gfmKey_esc,
    gfmKey_tab,
    gfmKey_space,
    gfmKey_return,
    gfmKey_backspace,
    gfmKey_left,
    gfmKey_right,
    gfmKey_up,
    gfmKey_down,
    gfmKey_insert,
    gfmKey_delete,
    gfmKey_home,
    gfmKey_end,
    gfmKey_pageUp,
    gfmKey_pageDown,
    /** Mouse button */
    gfmPointer_button,
    /** Gamepad buttons */
    gfmController_left,
    gfmController_right,
    gfmController_up,
    gfmController_down,
    gfmController_a,
    gfmController_b,
    gfmController_x,
    gfmController_y,
    gfmController_l1,
    gfmController_l2,
    gfmController_l3,
    gfmController_r1,
    gfmController_r2,
    gfmController_r3,
    gfmController_laxis_left,
    gfmController_laxis_right,
    gfmController_laxis_up,
    gfmController_laxis_down,
    gfmController_raxis_left,
    gfmController_raxis_right,
    gfmController_raxis_up,
    gfmController_raxis_down,
    gfmController_start,
    gfmController_select,
    gfmController_home,
    /** Controller's axis - NOTE: These are never triggered! */
    gfmController_leftAnalog,
    gfmController_rightAnalog,
    gfmController_leftTrigger,
    gfmController_rightTrigger,
    gfmController_leftAnalogX,
    gfmController_leftAnalogY,
    gfmController_rightAnalogX,
    gfmController_rightAnalogY,
    gfmIface_max
};
typedef enum enGFMInputIface gfmInputIface;

#endif /* __GFMINPUT_STRUCT__ */

#ifndef __GFMINPUT_H__
#define __GFMINPUT_H__

#include <GFraMe/gfmError.h>

/**
 * Alloc a new gfmInput context
 * 
 * @param  ppCtx The context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmInput_getNew(gfmInput **ppCtx);

/**
 * Free and clean up a gfmInput context
 * 
 * @param  ppCtx The context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_free(gfmInput **ppCtx);

/**
 * Initializes the gfmInput
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_init(gfmInput *pCtx);

/**
 * Cleans any memory use by the gfmInput
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_clean(gfmInput *pCtx);

/**
 * Set how long between presses for an multi-action to trigger; This is
 * dependant on the backend, but since the event system should be triggered
 * asynchronously from update/draw loops, this time needn't be aligned to the
 * game's FPS.
 * 
 * @param  pCtx The context
 * @param  ms   The time between presses
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_setMultiDelay(gfmInput *pCtx, unsigned int ms);

/**
 * Set the minimum value to detected a trigger/axis press
 * 
 * @param  pCtx The context
 * @param  val  The value to detect an axis as pressed
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_setAxisTrigger(gfmInput *pCtx, float val);

/**
 * Updates every input, correctly marking 'em  as just pressed or whatever
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_update(gfmInput *pCtx);

/**
 * Removes every bound key, so it all can be re-created
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_reset(gfmInput *pCtx);

/**
 * Adds a new virtual key to the context;
 * The handles are sequentily assigned, starting at 0
 * 
 * @param  pHandle Handle to the action
 * @param  pCtx    The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmInput_addVirtualKey(int *pHandle, gfmInput *pCtx);

/**
 * Bind a keyboard's key to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The key
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindKey(gfmInput *pCtx, int handle, gfmInputIface key);

/**
 * Bind a gamepad button to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  button The button
 * @param  port   Index of the gamepad to trigger this action; If there's no
 *                gamepad with the requested index, this will simply never
 *                triggers (this can be related to a console's port numbers);
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindButton(gfmInput *pCtx, int handle, gfmInputIface button,
        int port);

/**
 * Set the pointer's position (used internally)
 * 
 * @param  pCtx The context
 * @param  x    The pointer's horizontal position
 * @param  y    The pointer's vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_setPointerPosition(gfmInput *pCtx, int x, int y);

/**
 * Get the pointer's position
 * 
 * @param  pX   The pointer's horizontal position
 * @param  pY   The pointer's vertical position
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_getPointerPosition(int *pX, int *pY, gfmInput *pCtx);

/**
 * Get the current state of a gamepad's analog stick (i.e., poll the value)
 * 
 * @param  pX     The horizontal axis
 * @param  pY     The vertical axis
 * @param  pCtx   The input context
 * @param  port   The gamepad's port
 * @param  analog Which of the analog sticks to check
 * @return        GFMRV_OK, ...
 */
gfmRV gfmInput_getGamepadAnalog(double *pX, double *pY, gfmInput *pCtx, int port,
        gfmInputIface analog);

/**
 * Set the current value of a single axis from an analog stick
 * 
 * @param  pCtx       The input context
 * @param  port       The gamepad's port
 * @param  analogAxis The axis (and analog) being set
 * @param  val        The axis' new value
 * @param  time       Time, in milliseconds, when the event happened
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmInput_setGamepadAxis(gfmInput *pCtx, int port,
        gfmInputIface analogAxis, double val, unsigned int time);

/**
 * Swtch a key's state
 * 
 * @param  pCtx  The input context
 * @param  key   The key/button
 * @param  state The virtual key's new state
 * @param  time  Time, in milliseconds, when the event happened
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_KEY_NOT_BOUND,
 *               GFMRV_INPUT_INVALID_STATE
 */
gfmRV gfmInput_setKeyState(gfmInput *pCtx, gfmInputIface key,
        gfmInputState state, unsigned int time);

/**
 * Swtch a gamepad button's state
 * 
 * @param  pCtx   The input context
 * @param  button The key/button
 * @param  port   The controller port
 * @param  state  The virtual key's new state
 * @param  time   Time, in milliseconds, when the event happened
 * @param         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_KEY_NOT_BOUND,
 *                GFMRV_INPUT_INVALID_STATE
 */
gfmRV gfmInput_setButtonState(gfmInput *pCtx, gfmInputIface button, int port,
        gfmInputState state, unsigned int time);

/**
 * Retrieves a virtual key state
 * 
 * @param  pState The current state
 * @param  pNum   How many consecutive times the key has been pressed
 * @param  pCtx   The input context
 * @param  handle The action's handle
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE
 */
gfmRV gfmInput_getKeyState(gfmInputState *pState, int *pNum, gfmInput *pCtx,
        int handle);

/**
 * Get the last key pressed; This function must be called after
 * 'gfmInput_requestLastPressed', because it won't be able to correctly detect
 * a key press correctly; Also, note that this function won't block!
 * 
 * @param  pIface The pressed key
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_NOT_ACTIVE,
 *                GFMRV_WAITING
 */
gfmRV gfmInput_getLastPressed(gfmInputIface *pIface, gfmInput *pCtx);

/**
 * Get the port of the last pressed button; If the last input didn't come from
 * a gamepad, the port will be -1
 * NOTE: This function must be called before getLastPressed!!!
 * 
 * @param pPort The port
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_NOT_ACTIVE,
 *                GFMRV_WAITING
 */
gfmRV gfmInput_getLastPort(int *pPort, gfmInput *pCtx);

/**
 * Request that the next key pressed be store
 * 
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_requestLastPressed(gfmInput *pCtx);

#endif /* __GFMINPUT_H__ */

