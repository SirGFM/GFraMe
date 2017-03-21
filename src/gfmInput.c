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

/** All axis of a gamepad */
struct stGFMGamepadAxis {
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    float leftTrigger;
    float rightTrigger;
};
typedef struct stGFMGamepadAxis gfmGamepadAxis;

/** Input context */
struct stGFMInput {
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
    /** Minimum value to detect an axis as pressed */
    float axisTriggerVal;
    /** Whether this context is expecting a key press */
    int waitingInput;
    /** Last 'iface' pressed */
    gfmInputIface lastIface;
    /** Port of the last controller that pressed a button or -1 */
    int lastPort;
    /** Value of all axis, ordered by port */
    gfmGamepadAxis *pAxis;
    int pAxisLen;
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
    rv = gfmInput_setAxisTrigger(pCtx, 0.3f);
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
    // Free all the axis values
    if (pCtx->pAxis) {
        free(pCtx->pAxis);
        pCtx->pAxis = 0;
    }
    pCtx->pAxisLen = 0;
    
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
 * Set the minimum value to detected a trigger/axis press
 * 
 * @param  pCtx The context
 * @param  val  The value to detect an axis as pressed
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_setAxisTrigger(gfmInput *pCtx, float val) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    pCtx->axisTriggerVal = val;
    
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
        gfmInput_updateVKey(pCtx, i);
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Updates a single virtual key
 *
 * @param  [ in]pCtx The context
 * @param  [ in]vkey The key to be updated
 * @return      GFrame return value
 */
gfmRV gfmInput_updateVKey(gfmInput *pCtx, int vkey) {
    gfmVirtualKey *pVKey;
    gfmRV rv;

    ASSERT(vkey < gfmGenArr_getUsed(pCtx->pVKeys), GFMRV_INVALID_INDEX);

    pVKey = gfmGenArr_getObject(pCtx->pVKeys, vkey);

#define CHECK(test) \
    ((pVKey->state & (test)) == (test))

    if (CHECK(gfmInput_justPressed << gfmInput_forceBits)) {
        /* Force the next state to be pressed */
        pVKey->state &= ~(gfmInput_forceFrame | gfmInput_curFrame);
        pVKey->state |= gfmInput_justPressed;
        /* If the next state is pressed, clear it (since it was already set) */
        if (CHECK(gfmInput_justPressed << gfmInput_nextBits)) {
            pVKey->state &= ~gfmInput_nextFrame;
        }
    }
    else if (pVKey->state & gfmInput_nextFrame) {
        gfmInputState next;

        /* Switch the state to whatever was set */
        next = (pVKey->state & gfmInput_nextFrame) >> gfmInput_nextBits;

        pVKey->state &= ~(gfmInput_nextFrame | gfmInput_curFrame);
        pVKey->state |= next;
    }
    else if (pVKey->state & gfmInput_justMask) {
        /* After 1 frame with the just* bit set, clear it */
        pVKey->state &= ~gfmInput_justMask;
    }

#undef CHECK

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
gfmRV gfmInput_getGamepadAnalog(float *pX, float *pY, gfmInput *pCtx, int port,
        gfmInputIface analog) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(analog >= gfmController_leftAnalog, GFMRV_ARGUMENTS_BAD);
    ASSERT(analog <= gfmController_rightTrigger, GFMRV_ARGUMENTS_BAD);
    
    // Check if there's a connected controller on that port
    if (port >= pCtx->pAxisLen) {
        *pX = 0.0f;
        *pY = 0.0f;
    }
    else {
        // Retrieve the value from the requested axis
        switch (analog) {
            case gfmController_leftAnalog: {
                *pX = pCtx->pAxis[port].leftX;
                *pY = pCtx->pAxis[port].leftY;
            } break;
            case gfmController_rightAnalog: {
                *pX = pCtx->pAxis[port].rightX;
                *pY = pCtx->pAxis[port].rightY;
            } break;
            case gfmController_leftTrigger: {
                *pX = pCtx->pAxis[port].leftTrigger;
                *pY = pCtx->pAxis[port].leftTrigger;
            } break;
            case gfmController_rightTrigger: {
                *pX = pCtx->pAxis[port].rightTrigger;
                *pY = pCtx->pAxis[port].rightTrigger;
            } break;
            default: {}
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmInputIface analogAxis, double val, unsigned int time) {
    gfmRV rv;
    gfmInputIface posBt, negBt;
    gfmInputState posSt, negSt;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(analogAxis >= gfmController_leftTrigger, GFMRV_ARGUMENTS_BAD);
    ASSERT(analogAxis <= gfmController_rightAnalogY, GFMRV_ARGUMENTS_BAD);
    ASSERT(port >= 0, GFMRV_ARGUMENTS_BAD);
    
    // Check that the axis buffer is big enough to store this port's values
    if (pCtx->pAxisLen <= port) {
        pCtx->pAxis = (gfmGamepadAxis*)realloc(pCtx->pAxis,
                sizeof(gfmGamepadAxis) * (port + 1));
        ASSERT(pCtx->pAxis, GFMRV_ALLOC_FAILED);
        
        memset(&(pCtx->pAxis[port]), 0x0, sizeof(gfmGamepadAxis) *
                (port + 1 - pCtx->pAxisLen));
        
        pCtx->pAxisLen = port + 1;
    }
    
    // Retrieve the axis' button and store its value
    switch (analogAxis) {
        case gfmController_leftAnalogX: {
            posBt = gfmController_laxis_right;
            negBt = gfmController_laxis_left;
            pCtx->pAxis[port].leftX = val;
        } break;
        case gfmController_leftAnalogY: {
            posBt = gfmController_laxis_down;
            negBt = gfmController_laxis_up;
            pCtx->pAxis[port].leftY = val;
        } break;
        case gfmController_rightAnalogX: {
            posBt = gfmController_raxis_right;
            negBt = gfmController_raxis_left;
            pCtx->pAxis[port].rightX = val;
        } break;
        case gfmController_rightAnalogY: {
            posBt = gfmController_raxis_down;
            negBt = gfmController_raxis_up;
            pCtx->pAxis[port].rightY = val;
        } break;
        case gfmController_leftTrigger: {
            posBt = gfmController_l2;
            negBt = gfmIface_none;
            pCtx->pAxis[port].leftTrigger = val;
        } break;
        case gfmController_rightTrigger: {
            posBt = gfmController_r2;
            negBt = gfmIface_none;
            pCtx->pAxis[port].rightTrigger = val;
        } break;
        default: {}
    }
    
    // If the key isn't bound, exit (since the value was already stored)
    if (pCtx->pTree == 0) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    // Set each direction's state
    if (val > pCtx->axisTriggerVal) {
        posSt = gfmInput_justPressed;
        negSt = gfmInput_justReleased;
    }
    else if (val < -pCtx->axisTriggerVal) {
        posSt = gfmInput_justReleased;
        negSt = gfmInput_justPressed;
    }
    else {
        posSt = gfmInput_justReleased;
        negSt = gfmInput_justReleased;
    }
    
    // Set both direction's values
    rv = gfmInput_setButtonState(pCtx, posBt, port, posSt, time);
    ASSERT(rv == GFMRV_OK, rv);
    // Ignore the return on the negative direction
    gfmInput_setButtonState(pCtx, negBt, port, negSt, time);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a virtual key's state, considered its current state, its next one and
 * what was issued
 *
 * @param  [ in]pCtx  The input context
 * @param  [ in]pVKey The virtual key
 * @param  [ in]state The issued state
 * @param  [ in]time  When the state was issued
 */
static void _gfmInput_setVKeyState(gfmInput *pCtx, gfmVirtualKey *pVKey
        , gfmInputState state, unsigned int time) {
#define CASE(cur, next, received) \
    case (cur | (next << 4) | (received << 8))
    /* Try to neatly organize which case was detected:
     *
     * Bits   0-3: Current state
     * Bits   4-7: Next state
     * Bits  8-11: Issued state
     *
     */
    switch ((pVKey->state &
            (gfmInput_stateMask | (gfmInput_stateMask << gfmInput_nextBits)))
            | ((state & gfmInput_stateMask) << 8)) {
        CASE(gfmInput_pressed, 0, gfmInput_pressed):
        CASE(gfmInput_released, 0, gfmInput_released):
        CASE(gfmInput_pressed, gfmInput_pressed, gfmInput_pressed):
        CASE(gfmInput_pressed, gfmInput_released, gfmInput_released):
        CASE(gfmInput_released, gfmInput_pressed, gfmInput_pressed):
        CASE(gfmInput_released, gfmInput_released, gfmInput_released):
            /* Received a repeated event */
            break;
        CASE(gfmInput_pressed, gfmInput_released, gfmInput_pressed):
        CASE(gfmInput_released, gfmInput_released, gfmInput_pressed):
        CASE(gfmInput_released, 0, gfmInput_pressed):
        CASE(0, 0, gfmInput_pressed):
            /* Force the input to be pressed on the next frame */
            pVKey->state &= ~gfmInput_forceFrame;
            pVKey->state |= (state << gfmInput_forceBits);

            if (time - pVKey->lastPress <= pCtx->multiDelay) {
                pVKey->num++;
            }
            else {
                pVKey->num = 1;
            }
            pVKey->lastPress = time;
            /* NOTE: Fallthrough */
        CASE(gfmInput_pressed, gfmInput_pressed, gfmInput_released):
        CASE(gfmInput_released, gfmInput_pressed, gfmInput_released):
        CASE(gfmInput_pressed, 0, gfmInput_released):
        CASE(0, 0, gfmInput_released):
            /* Set the next frame with whatever was received (though it may
             * be overriden) */

            pVKey->state &= ~gfmInput_nextFrame;
            pVKey->state |= (state << gfmInput_nextBits);
            break;
        default: { /* Shouldn't happen */ }
    }
#undef CASE
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
    if (pCtx->pTree == 0) {
        rv = GFMRV_OK;
        goto __ret;
    }
    // TODO Assert the state?
    
    // Try to retrieve the bound virtual key
    rv = gfmKeyNode_getVirtualKey(&pVKey, pCtx->pTree, key);
    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_INPUT_NOT_BOUND);
    
    if (rv == GFMRV_OK) {
        _gfmInput_setVKeyState(pCtx, pVKey, state, time);
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
    //ASSERT(button < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    ASSERT(port >= 0, GFMRV_ARGUMENTS_BAD);
    // If there're no keys, return
    if (pCtx->pTree == 0) {
        rv = GFMRV_OK;
        goto __ret;
    }
    // TODO Assert the state?
    
    // 'Convert' the button, given the controller port
    realButton = button + (gfmController_home - gfmController_left) * port;
    
    // Try to retrieve the bound virtual key
    rv = gfmKeyNode_getVirtualKey(&pVKey, pCtx->pTree, realButton);
    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_INPUT_NOT_BOUND);
    
    // If the input is bound and this isn't a repeated press
    if (rv == GFMRV_OK) {
        _gfmInput_setVKeyState(pCtx, pVKey, state, time);
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
 * Get the port of the last pressed button; If the last input didn't come from
 * a gamepad, the port will be -1
 * NOTE: This function must be called before getLastPressed!!!
 * 
 * @param pPort The port
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_NOT_ACTIVE,
 *                GFMRV_WAITING
 */
gfmRV gfmInput_getLastPort(int *pPort, gfmInput *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPort, GFMRV_ARGUMENTS_BAD);
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
    
    // Set the return
    *pPort = pCtx->lastPort;
    
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

/**
 * Stop storing the last pressed key
 *
 * @param  pCtx   The input context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmInput_cancelRequestLastPressed(gfmInput *pCtx) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    pCtx->waitingInput = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

