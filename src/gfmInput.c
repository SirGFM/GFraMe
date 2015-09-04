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
    /** Whether this context is expecting a key press */
    int waitingInput;
    /** Last 'iface' pressed */
    gfmInputIface lastIface;
    /** Port of the last controller that pressed a button or -1 */
    int lastPort;
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
    // Removes every virtual key
    gfmGenArr_reset(pCtx->pVKeys);
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
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Update every virtual key
    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pVKeys)){
        gfmVirtualKey *pVKey;
        
        pVKey = gfmGenArr_getObject(pCtx->pVKeys, i);
        
        if ((pVKey->state & (gfmInput_justPressed << 8))
                == (gfmInput_justPressed << 8)) {
            // If the key was just pressed (it's set on byte 1!!)
            pVKey->state = pVKey->state & ~(gfmInput_justPressed << 8);
            pVKey->state = pVKey->state & ~gfmInput_curFrame;
            pVKey->state = pVKey->state | gfmInput_justPressed;
        }
        else if ((pVKey->state & (gfmInput_justReleased << 8))
                == (gfmInput_justReleased << 8)) {
            // If the key was just released (it's set on byte 1!!)
            pVKey->state = pVKey->state & ~(gfmInput_justReleased << 8);
            pVKey->state = pVKey->state & ~gfmInput_curFrame;
            pVKey->state = pVKey->state | gfmInput_justReleased;
        }
        else if ((pVKey->state & gfmInput_justPressed)
                == gfmInput_justPressed) {
            // If the key was pressed on the last frame
            pVKey->state = pVKey->state & ~gfmInput_curFrame;
            pVKey->state = pVKey->state | gfmInput_pressed;
        }
        else if ((pVKey->state & gfmInput_justReleased)
                == gfmInput_justReleased) {
            // If the key was released on the last frame
            pVKey->state = pVKey->state & ~gfmInput_curFrame;
            pVKey->state = pVKey->state | gfmInput_released;
        }
        
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Removes every bound key, so it all can be re-created
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_reset(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
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
 * Bind a keyboard's key to an action
 * 
 * @param  pCtx   The context
 * @param  handle The action's handle
 * @param  key    The key
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmInput_bindKey(gfmInput *pCtx, int handle, gfmInputIface key) {
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
        int port) {
    gfmKeyNode *pNode;
    gfmRV rv;
    gfmVirtualKey *pVKey;
    gfmInputIface realButton;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(button > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(button < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    ASSERT(port >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the handle is valid
    ASSERT(handle < gfmGenArr_getUsed(pCtx->pVKeys),
            GFMRV_INPUT_INVALID_HANDLE);
    
    // 'Convert' the button, given the controller port
    realButton = button + (gfmController_home - gfmController_left) * port;
    
    // Check that the button is still unbound
    rv = gfmKeyNode_isBound(pCtx->pTree, realButton);
    ASSERT(rv == GFMRV_FALSE, GFMRV_INPUT_ALREADY_BOUND);
    
    // Retrieve the handle's virtual key
    pVKey = gfmGenArr_getObject(pCtx->pVKeys, handle);
    
    // Get a new node
    gfmGenArr_getNextRef(gfmKeyNode, pCtx->pKeys, 1, pNode,
            gfmKeyNode_getNew);
    // Initialize it
    rv = gfmKeyNode_init(pNode, realButton, pVKey);
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
        gfmInputIface analog) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

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
        gfmInputState state, unsigned int time) {
    gfmRV rv;
    gfmVirtualKey *pVKey;
    
    // Satinize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(key > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(key < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    // If there're no keys, return
    ASSERT(pCtx->pTree, GFMRV_OK);
    // TODO Assert the state?
    
    // Try to retrieve the bound virtual key
    rv = gfmKeyNode_getVirtualKey(&pVKey, pCtx->pTree, key);
    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_INPUT_NOT_BOUND);
    
    // If the input is bound and this isn't a repeated press
    if (rv == GFMRV_OK && ((pVKey->state & state) & gfmInput_stateMask) == 0) {
        // Set the next state on byte 1
        pVKey->state |= state << 8;
        
        // Check for multi-press
        if ((state & gfmInput_pressed) == gfmInput_pressed) {
            if (time - pVKey->lastPress <= pCtx->multiDelay) {
                pVKey->num++;
            }
            else {
                pVKey->num = 1;
            }
            // Update the time
            pVKey->lastPress = time;
        }
    }
    
    // Store the last pressed key, if the operation is active
    if (pCtx->waitingInput && (state & gfmInput_pressed)) {
        pCtx->lastIface = key;
        pCtx->lastPort = -1;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmInputState state, unsigned int time) {
    gfmRV rv;
    gfmVirtualKey *pVKey;
    gfmInputIface realButton;
    
    // Satinize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(button > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(button < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    ASSERT(port >= 0, GFMRV_ARGUMENTS_BAD);
    // If there're no keys, return
    ASSERT(pCtx->pTree, GFMRV_OK);
    // TODO Assert the state?
    
    // 'Convert' the button, given the controller port
    realButton = button + (gfmController_home - gfmController_left) * port;
    
    // Try to retrieve the bound virtual key
    rv = gfmKeyNode_getVirtualKey(&pVKey, pCtx->pTree, realButton);
    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_INPUT_NOT_BOUND);
    
    // If the input is bound and this isn't a repeated press
    if (rv == GFMRV_OK && ((pVKey->state & state) & gfmInput_stateMask) == 0) {
        // Set the next state on byte 1
        pVKey->state |= state << 8;
        
        // Check for multi-press
        if ((state & gfmInput_pressed) == gfmInput_pressed) {
            if (time - pVKey->lastPress <= pCtx->multiDelay) {
                pVKey->num++;
            }
            else {
                pVKey->num = 1;
            }
            // Update the time
            pVKey->lastPress = time;
        }
    }
    
    // Store the last pressed key, if the operation is active
    if (pCtx->waitingInput && (state & gfmInput_pressed)) {
        pCtx->lastIface = button;
        pCtx->lastPort = port;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int handle) {
    gfmRV rv;
    gfmVirtualKey *pVKey;
    
    // Sanitize arguments
    ASSERT(pState, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNum, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the handle is valid
    ASSERT(handle < gfmGenArr_getUsed(pCtx->pVKeys),
            GFMRV_INPUT_INVALID_HANDLE);
    
    // Retrieve the virtual key
    pVKey = gfmGenArr_getObject(pCtx->pVKeys, handle);
    // Get its state
    *pState = pVKey->state & gfmInput_curFrame;
    *pNum = pVKey->num;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmInput_getLastPressed(gfmInputIface *pIface, gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pIface, GFMRV_ARGUMENTS_BAD);
    // Check that the operation was initialized
    if (!(pCtx->waitingInput)) {
        rv = GFMRV_OPERATION_NOT_ACTIVE;
        goto __ret;
    }
    // Check that a key was pressed
    if (pCtx->lastIface == gfmIface_none) {
        rv = GFMRV_WAITING;
        goto __ret;
    }
    
    // Deactivate the operation
    pCtx->waitingInput = 0;
    // Set the return
    *pIface = pCtx->lastIface;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Request that the next key pressed be store
 * 
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_requestLastPressed(gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Activate the request operation
    pCtx->waitingInput = 1;
    pCtx->lastIface = gfmIface_none;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

