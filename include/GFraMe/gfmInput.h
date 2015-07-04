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
    gfmInput_justMask     = 0x0C
};
typedef enum enGFMInputState gfmInputState;

/** Map to input interface; Either a keyboard key, a controller button or a
    mouse button; Note that the controller must be the last device, as a
    controller button is multiplied by its ID */
enum enGFMInputIface {
    gfmIface_none = 0,
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
    gfmPointer_button,
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
    gfmController_start,
    gfmController_select,
    gfmController_home,
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
 * Updates every input, correctly marking 'em  as just pressed or whatever
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_update(gfmInput *pCtx);

/**
 * Removes every virtual key and bound key, so it all can be re-created
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
 * Bind a key/button to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The key/button
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bind(gfmInput *pCtx, int handle, gfmInputIface key);

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
 * Swtch a key/button's state
 * 
 * @param  pCtx  The input context
 * @param  key   The key/button
 * @param  state The virtual key's new state
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_KEY_NOT_BOUND,
 *               GFMRV_INPUT_INVALID_STATE
 */
gfmRV gfmInput_setKeyState(gfmInput *pCtx, gfmInputIface key,
        gfmInputState state);

#if 0

/**
 * Add a new action to the context; Since this is expected to be more of a
 * static context (i.e., from the start of the game you know every possible
 * action that should be checked for), there's no way to remove actions after
 * they are added!
 * 
 * The handles are sequentily assigned, starting at 0
 * 
 * @param  pHandle Handle to the action
 * @param  pCtx    The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmInput_addAction(int *pHandle, gfmInput *pCtx);

/**
 * Removes all active bindings from an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE
 */
gfmRV gfmInput_cleanAction(gfmInput *pCtx, int handle);

/**
 * Checks an action's current status
 * 
 * @param  pStatus The action's status
 * @param  pCtx    The context
 * @param  handle  The action's handle
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE
 */
gfmRV gfmInput_getAction(gfmInputStatus *pStatus, gfmInput *pCtx, int handle);

/**
 * Unbind a keyboard key from an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The keyboard key
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindKey(gfmInput *pCtx, int handle, gfmInputKey key);

/**
 * Bind a keyboard key to an action, if it's pressed a few consecutive times
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The keyboard key
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindMultiKey(gfmInput *pCtx, int handle, gfmInputKey key,
        int num);

/**
 * Unbind a keyboard key from an action that requires consecutives presses
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The keyboard key
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindMultiKey(gfmInput *pCtx, int handle, gfmInputKey key,
        int num);

/**
 * Bind a mouse click/touch to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindPointer(gfmInput *pCtx, int handle);

/**
 * Unbind a mouse click/touch from an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindPointer(gfmInput *pCtx, int handle);

/**
 * Bind a mouse click/touch to an action, if it's pressed a few consecutive
 * times
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindMultiPointer(gfmInput *pCtx, int handle, int num);

/**
 * Unbind a mouse click/touch from an action that requires consecutives presses
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindMultiPointer(gfmInput *pCtx, int handle, int num);

/**
 * Get how many controllers are available
 * 
 * @param  pNum The number of controllers
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_getNumControllers(int *pNum, gfmInput *pCtx);

/**
 * Updates the list of available cotrollers (used internally only)
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_updateControllers(gfmInput *pCtx);

/**
 * Bind a controller's button to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  bt     The controller's button
 * @param  index  Which controller should be used
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindController(gfmInput *pCtx, int handle, gfmInputController bt,
        int index);
/**
 * Unbind a controller's button from an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  bt     The controller's button
 * @param  index  Which controller should be used
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindController(gfmInput *pCtx, int handle,
        gfmInputController bt, int index);

/**
 * Bind a controller's button to an action, if it's pressed a few consecutive
 * times
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  bt     The controller's button
 * @param  index  Which controller should be used
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindMultiController(gfmInput *pCtx, int handle,
        gfmInputController bt, int index, int num);
/**
 * Unbind a controller's button from an action that requires consecutives
 * presses
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  bt     The controller's button
 * @param  index  Which controller should be used
 * @param  num    Number of consecutive presses to activate the action
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_NOT_BOUND
 */
gfmRV gfmInput_unbindMultiController(gfmInput *pCtx, int handle,
        gfmInputController bt, int index, int num);

#endif /* 0 */

#endif /* __GFMINPUT_H__ */

