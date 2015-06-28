/**
 * @file src/include/GFraMe_int/gfmInput.h
 * 
 * Generic input interface; Acts as frontend for the gfmEvent backend; Instead
 * of giving access to each possible input device, this maps inputs to actions,
 * easing customizable controls; A drawback to this implementation is that it
 * forces the user to map each key before using them (but that also forces 'em
 * to use it correctly!)
 * 
 * Each action may be either 'pressed' or 'released', but there is a flag to
 * signal the frame when the action switched; Also, actions may be assigned to
 * a controller's button; Be aware that as soon as a controller is
 * disconected, its bindings will be released
 * Note that whenever a single and multi-action input is added the library will
 * have to decide between the two types, and only one shall be marked. Therefore
 * this may result in some slight lag.
 * 
 * Eventually, there'll be a special type of action to get the axis value of a
 * controller; For now, if an axis is used as input it will have a deadzone of
 * 30% of it's full motion
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmInput.h>

#include <stdlib.h>
#include <string.h>

struct enGFMInput {
    /** Last/current pointer position (already in screen-space) */
    int pointerX;
    /** Last/current pointer position (already in screen-space) */
    int pointerY;
    int null;
};

/**
 * Alloc a new gfmInput context
 * 
 * @param  ppCtx The context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmInput_getNew(gfmInput **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and clean the object
    *ppCtx = (gfmInput*)malloc(sizeof(gfmInput));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmInput));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free and clean up a gfmInput context
 * 
 * @param  ppCtx The context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_free(gfmInput **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the object and release it
    gfmInput_clean(*ppCtx);
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initializes the gfmInput
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_init(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // TODO
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Cleans any memory use by the gfmInput
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_clean(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // TODO
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmInput_setMultiDelay(gfmInput *pCtx, int ms);

/**
 * Updates every input, correctly marking 'em  as just pressed or whatever
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_update(gfmInput *pCtx);

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
 * Bind a keyboard key to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The keyboard key
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindKey(gfmInput *pCtx, int handle, gfmInputKey key);

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
 * Set the pointer's position (used internally)
 * 
 * @param  pCtx The context
 * @param  x    The pointer's horizontal position
 * @param  y    The pointer's vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_setPointerPosition(gfmInput *pCtx, int x, int y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Store the position
    pCtx->pointerX = x;
    pCtx->pointerY = y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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

