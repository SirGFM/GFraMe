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
#include <GFraMe_int/gfmKeyNode.h>
#include <GFraMe_int/gfmVirtualKey.h>

#include <stdlib.h>
#include <string.h>

/** Creates the virtual key array type */
gfmGenArr_define(gfmVirtualKey);
/** Creates the nodes array type */
gfmGenArr_define(gfmKeyNode);

/** Input context */
struct enGFMInput {
    /** Expandable array of virtual keys */
    gfmGenArr_var(gfmVirtualKey, pVKeys);
    /** Expandable array with all keys that form the tree of bound keys */
    gfmGenArr_var(gfmKeyNode, pKeys);
    /** Binary tree of bound keys */
    gfmKeyNode *pTree;
    /** Last/current pointer position (already in screen-space) */
    int pointerX;
    /** Last/current pointer position (already in screen-space) */
    int pointerY;
    /** How long a between 'presses' in a multi-press (in ms) */
    unsigned int multiDelay;
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
    unsigned int ms;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set how long between multi-presses (defaults to 300 ms)
    ms = 300;
    rv = gfmInput_setMultiDelay(pCtx, ms);
    ASSERT_NR(rv == GFMRV_OK);
    // Pre-allocate the virtual key array
    gfmGenArr_setMinSize(gfmVirtualKey, pCtx->pVKeys, 14, gfmVirtualKey_getNew);
    // Pre-allocate some nodes
    gfmGenArr_setMinSize(gfmKeyNode, pCtx->pKeys, 28, gfmKeyNode_getNew);
    // Clear everything
    rv = gfmInput_reset(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
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
    
    // Release every previously allocated virtual key
    gfmGenArr_clean(pCtx->pVKeys, gfmVirtualKey_free);
    gfmGenArr_clean(pCtx->pKeys, gfmKeyNode_free);
    pCtx->pTree = 0;
    
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
gfmRV gfmInput_setMultiDelay(gfmInput *pCtx, unsigned int ms) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    pCtx->multiDelay = ms;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Updates every input, correctly marking 'em  as just pressed or whatever
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_update(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // TODO Update actions
    
    // Update the current pointer status
    /*
    pCtx->pointer.state = pCtx->pointer.state & (~gfmInput_justMask);
    if (pCtx->pointer.state == gfmInput_released) {
        pCtx->pointer.num = 0;
    }
    */
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Removes every virtual key and bound key, so it all can be re-created
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_reset(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Removes every virtual key
    gfmGenArr_reset(pCtx->pVKeys);
    // Removes every node from the tree
    gfmGenArr_reset(pCtx->pKeys);
    // Reset the tree's root
    pCtx->pTree = 0;
    // Reset the pointer back to the origin (0,0)
    pCtx->pointerX = 0;
    pCtx->pointerY = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Adds a new virtual key to the context;
 * The handles are sequentily assigned, starting at 0
 * 
 * @param  pHandle Handle to the action
 * @param  pCtx    The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmInput_addVirtualKey(int *pHandle, gfmInput *pCtx) {
    gfmRV rv;
    gfmVirtualKey *pTmp;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHandle, GFMRV_ARGUMENTS_BAD);
    
    // Get a new virtual key from the array
    gfmGenArr_getNextRef(gfmVirtualKey, pCtx->pVKeys, 1, pTmp,
            gfmVirtualKey_getNew);
    rv = gfmVirtualKey_init(pTmp);
    // Get this key's handle
    *pHandle = gfmGenArr_getUsed(pCtx->pVKeys);
    // 'Push' it into the array
    gfmGenArr_push(pCtx->pVKeys);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Bind a key/button to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The key/button
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bind(gfmInput *pCtx, int handle, gfmInputIface key) {
    gfmKeyNode *pNode;
    gfmRV rv;
    gfmVirtualKey *pVKey;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(key > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(key < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    // Check that the handle is valid
    ASSERT(handle < gfmGenArr_getUsed(pCtx->pVKeys),
            GFMRV_INPUT_INVALID_HANDLE);
    
    // Check that the key is still unbound
    rv = gfmKeyNode_isBound(pCtx->pTree, key);
    ASSERT(rv == GFMRV_FALSE, GFMRV_INPUT_ALREADY_BOUND);
    
    // Retrieve the handle's virtual key
    pVKey = gfmGenArr_getObject(pCtx->pVKeys, handle);
    
    // Get a new node
    gfmGenArr_getNextRef(gfmKeyNode, pCtx->pKeys, 1, pNode,
            gfmKeyNode_getNew);
    // Initialize it
    rv = gfmKeyNode_init(pNode, key, pVKey);
    ASSERT_NR(rv == GFMRV_OK);
    // And insert it into the tree
    rv = gfmKeyNode_insert(pNode, &(pCtx->pTree));
    ASSERT_NR(rv == GFMRV_OK);
    
    // 'Push' it into the array
    gfmGenArr_push(pCtx->pKeys);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmInput_getPointerPosition(int *pX, int *pY, gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the position
    *pX = pCtx->pointerX;
    *pY = pCtx->pointerY;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmInputState state) {
    gfmRV rv;
    gfmVirtualKey *pVKey;
    
    // Satinize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(key > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(key < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    // TODO Assert the state?
    
    // Try to retrieve the bound virtual key
    rv = gfmKeyNode_getVirtualKey(&pVKey, pCtx->pTree, key);
    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_INPUT_NOT_BOUND);
    
    // If the input wasn't bound, do nothing!
    if (rv == GFMRV_OK) {
        pVKey->state = state;
        // TODO Check for multi-press
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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

gfmRV gfmInput_setPointerState(gfmInput *pCtx, gfmInputState state, unsigned int time) {
    gfmRV rv;
    unsigned int delay;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(state == gfmInput_justReleased || state == gfmInput_justPressed,
            GFMRV_ARGUMENTS_BAD);
    
    // Check if a multi-press is happening
    delay = time - pCtx->pointer.lastTime;
    if (state == gfmInput_justPressed && delay < pCtx->multiDelay) {
        pCtx->pointer.num++;
    }
    else if (state == gfmInput_justPressed) {
        // Otherwise, set it back to 1
        pCtx->pointer.num = 1;
    }
    // Set the pointer state
    pCtx->pointer.state = state;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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

