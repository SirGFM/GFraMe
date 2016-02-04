/**
 * @file src/core/event/android/gfmEvent_android.h
 * 
 * Module for managing events on Android
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/core/gfmEvent_bkend.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_timer.h>

#include <stdlib.h>
#include <string.h>

SDL_GameController *c;

/** The gfmEvent structure */
struct stGFMEvent {
    /** The last time accumulated */
    unsigned int accLastTime;
    /** Event that will be pushed on every timer callback */
    SDL_Event accTimerEvent;
    /** Array of connected and open controllers */
    SDL_GameController **pGamepads;
    /** Array with the controllers' ids */
    int *pGamepadIDs;
    /** Controller's array length */
    int numControllers;
};

/** Size of gfmEvent */
const int sizeofGFMEvent = (int)sizeof(gfmEvent);

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Converts a SDL button to it's gfmIface mapping
 * 
 * @param  button The SDL2 button
 * @return        The respective interface
 */
static gfmInputIface st_gfmEvent_convertSDLButton2GFM(Uint8 button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A: return gfmController_a; break;
        case SDL_CONTROLLER_BUTTON_B: return gfmController_b; break;
        case SDL_CONTROLLER_BUTTON_X: return gfmController_x; break;
        case SDL_CONTROLLER_BUTTON_Y: return gfmController_y; break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return gfmController_l1; break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return gfmController_r1; break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return gfmController_l3; break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return gfmController_r3; break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return gfmController_up; break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return gfmController_down; break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return gfmController_left; break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return gfmController_right; break;
        case SDL_CONTROLLER_BUTTON_BACK: return gfmController_select; break;
        case SDL_CONTROLLER_BUTTON_GUIDE: return gfmController_home; break;
        case SDL_CONTROLLER_BUTTON_START: return gfmController_start; break;
        default: return gfmIface_none;
    }
}

/**
 * Converts a SDL keycode to it's gfmIface mapping
 * 
 * @param  sym The key
 * @return     The respective interface
 */
static gfmInputIface st_gfmEvent_convertSDLKey2GFM(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_RETURN: return gfmKey_return;
        case SDLK_ESCAPE: return gfmKey_esc;
        case SDLK_BACKSPACE: return gfmKey_backspace;
        case SDLK_TAB: return gfmKey_tab;
        case SDLK_SPACE: return gfmKey_space;
                         /*
                            SDLK_EXCLAIM = '!',
                            SDLK_QUOTEDBL = '"',
                            SDLK_HASH = '#',
                            SDLK_PERCENT = '%',
                            SDLK_DOLLAR = '$',
                            SDLK_AMPERSAND = '&',
                            SDLK_QUOTE = '\'',
                            SDLK_LEFTPAREN = '(',
                            SDLK_RIGHTPAREN = ')',
                            SDLK_ASTERISK = '*',
                            SDLK_PLUS = '+',
                            SDLK_COMMA = ',',
                            SDLK_MINUS = '-',
                            SDLK_PERIOD = '.',
                            SDLK_SLASH = '/',
                          */
        case SDLK_0: return gfmKey_0;
        case SDLK_1: return gfmKey_1;
        case SDLK_2: return gfmKey_2;
        case SDLK_3: return gfmKey_3;
        case SDLK_4: return gfmKey_4;
        case SDLK_5: return gfmKey_5;
        case SDLK_6: return gfmKey_6;
        case SDLK_7: return gfmKey_7;
        case SDLK_8: return gfmKey_8;
        case SDLK_9: return gfmKey_9;
                     /*
                        SDLK_COLON = ':',
                        SDLK_SEMICOLON = ';',
                        SDLK_LESS = '<',
                        SDLK_EQUALS = '=',
                        SDLK_GREATER = '>',
                        SDLK_QUESTION = '?',
                        SDLK_AT = '@',
                        SDLK_LEFTBRACKET = '[',
                        SDLK_BACKSLASH = '\\',
                        SDLK_RIGHTBRACKET = ']',
                        SDLK_CARET = '^',
                        SDLK_UNDERSCORE = '_',
                        SDLK_BACKQUOTE = '`',
                      */
        case SDLK_a: return gfmKey_a;
        case SDLK_b: return gfmKey_b;
        case SDLK_c: return gfmKey_c;
        case SDLK_d: return gfmKey_d;
        case SDLK_e: return gfmKey_e;
        case SDLK_f: return gfmKey_f;
        case SDLK_g: return gfmKey_g;
        case SDLK_h: return gfmKey_h;
        case SDLK_i: return gfmKey_i;
        case SDLK_j: return gfmKey_j;
        case SDLK_k: return gfmKey_k;
        case SDLK_l: return gfmKey_l;
        case SDLK_m: return gfmKey_m;
        case SDLK_n: return gfmKey_n;
        case SDLK_o: return gfmKey_o;
        case SDLK_p: return gfmKey_p;
        case SDLK_q: return gfmKey_q;
        case SDLK_r: return gfmKey_r;
        case SDLK_s: return gfmKey_s;
        case SDLK_t: return gfmKey_t;
        case SDLK_u: return gfmKey_u;
        case SDLK_v: return gfmKey_v;
        case SDLK_w: return gfmKey_w;
        case SDLK_x: return gfmKey_x;
        case SDLK_y: return gfmKey_y;
        case SDLK_z: return gfmKey_z;
        case SDLK_F1: return gfmKey_f1;
        case SDLK_F2: return gfmKey_f2;
        case SDLK_F3: return gfmKey_f3;
        case SDLK_F4: return gfmKey_f4;
        case SDLK_F5: return gfmKey_f5;
        case SDLK_F6: return gfmKey_f6;
        case SDLK_F7: return gfmKey_f7;
        case SDLK_F8: return gfmKey_f8;
        case SDLK_F9: return gfmKey_f9;
        case SDLK_F10: return gfmKey_f10;
        case SDLK_F11: return gfmKey_f11;
        case SDLK_F12: return gfmKey_f12;
        case SDLK_INSERT: return gfmKey_insert;
        case SDLK_HOME: return gfmKey_home;
        case SDLK_PAGEUP: return gfmKey_pageUp;
        case SDLK_DELETE: return gfmKey_delete;
        case SDLK_END: return gfmKey_end;
        case SDLK_PAGEDOWN: return gfmKey_pageDown;
        case SDLK_RIGHT: return gfmKey_right;
        case SDLK_LEFT: return gfmKey_left;
        case SDLK_DOWN: return gfmKey_down;
        case SDLK_UP: return gfmKey_up;
                      /*
                         SDLK_KP_DIVIDE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
                         SDLK_KP_MULTIPLY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
                         SDLK_KP_MINUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
                         SDLK_KP_PLUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
                       */
        case SDLK_KP_ENTER: return gfmKey_nReturn;
        case SDLK_KP_1: return gfmKey_n1;
        case SDLK_KP_2: return gfmKey_n2;
        case SDLK_KP_3: return gfmKey_n3;
        case SDLK_KP_4: return gfmKey_n4;
        case SDLK_KP_5: return gfmKey_n5;
        case SDLK_KP_6: return gfmKey_n6;
        case SDLK_KP_7: return gfmKey_n7;
        case SDLK_KP_8: return gfmKey_n8;
        case SDLK_KP_9: return gfmKey_n9;
        case SDLK_KP_0: return gfmKey_n0;
                        /*
                           SDLK_KP_PERIOD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),
                         */
        case SDLK_LCTRL: return gfmKey_lctrl;
        case SDLK_LSHIFT: return gfmKey_lshift;
        case SDLK_LALT: return gfmKey_lalt;
        case SDLK_RCTRL: return gfmKey_rctrl;
        case SDLK_RSHIFT: return gfmKey_rshift;
        case SDLK_RALT: return gfmKey_ralt;
        default: return gfmIface_none;
    }
}

/**
 * Bind all currently connected controllers
 * 
 * @param  pCtx The event context
 * @param  pLog The logging context
 */
static gfmRV gfmEvent_bindAllControllers(gfmEvent *pCtx, gfmLog *pLog) {
    gfmRV rv;
    int i, num;
    
    // Sanitize arguments
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_debug, "Binding all controllers...");
    ASSERT(rv == GFMRV_OK, rv);
    
    // Get how many controllers there are
    num = SDL_NumJoysticks();
    rv = gfmLog_log(pLog, gfmLog_debug, "  There are %i controllers to be bound"
            "...", num);
    ASSERT(rv == GFMRV_OK, rv);
    
    rv = gfmLog_log(pLog, gfmLog_debug, "  Expanding controllers array to %i",
            num);
    ASSERT(rv == GFMRV_OK, rv);
    
    // Expand the game controllers' buffer
    pCtx->pGamepads = (SDL_GameController**)realloc(pCtx->pGamepads,
            num * sizeof(SDL_GameController**));
    ASSERT_LOG(pCtx->pGamepads, GFMRV_ALLOC_FAILED, pLog);
    pCtx->pGamepadIDs = (int*)realloc(pCtx->pGamepadIDs, num * sizeof(int));
    ASSERT_LOG(pCtx->pGamepadIDs, GFMRV_ALLOC_FAILED, pLog);
    // Clear the expanded part of the array
    memset(&(pCtx->pGamepads[pCtx->numControllers]), 0x00,
            sizeof(SDL_GameController**) * (num - pCtx->numControllers));
    memset(&(pCtx->pGamepadIDs[pCtx->numControllers]), 0xFF,
            sizeof(int) * (num - pCtx->numControllers));
    
    // Store the new length
    pCtx->numControllers = num;
    
    // Try to bind all new controllers
    i = 0;
    while (i < num) {
        SDL_GameController *pC;
        SDL_Joystick *pJ;
        //SDL_JoystickGUID guid;
        //char pGuidStr[33];
        int id;
        
        // Try to open the controller
        pC = SDL_GameControllerOpen(i);
        // TODO If this fails, check if there's a mapping for the device's GUID,
        // load it and then try again
        ASSERT_LOG(pC, GFMRV_CONTROLLER_FAILED_TO_BIND, pLog);
        
        // Get its guid (so we can log)
        //guid = SDL_JoystickGetDeviceGUID(i);
        //SDL_JoystickGetGUIDString(guid, pGuidStr, 33);
        
        // Get its ID (so we now its order)
        pJ = SDL_GameControllerGetJoystick(pC);
        ASSERT_LOG(pJ, GFMRV_INTERNAL_ERROR, pLog);
        id = (int)SDL_JoystickInstanceID(pJ);
        ASSERT_LOG(id >= 0, GFMRV_CONTROLLER_INVALID_ID, pLog);
        
        rv = gfmLog_log(pLog, gfmLog_debug, "  Bound Controller %i to index %i",
                id, i);
        ASSERT(rv == GFMRV_OK, rv);
        //rv = gfmLog_log(pLog, gfmLog_debug, "  Bound Controller %*s to index %i",
        //        33, pGuidStr, id);
        //ASSERT(rv == GFMRV_OK, rv);
        
        // Store it and its id
        pCtx->pGamepads[i] = pC;
        pCtx->pGamepadIDs[i] = id;
        
        i++;
    }
    
    rv = gfmLog_log(pLog, gfmLog_debug, "Done binding controllers!");
    ASSERT(rv == GFMRV_OK, rv);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Unbind all currently opened/bound controllers
 * 
 * @param  pCtx The event context
 */
static gfmRV gfmEvent_unbindAllControllers(gfmEvent *pCtx) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove all controllers
    i = 0;
    while (i < pCtx->numControllers) {
        SDL_GameController *pC;
        
        // If a controller, if any, on the current index
        pC = pCtx->pGamepads[i];
        if (pC) {
            // Yay! It doesn't return anything!
            SDL_GameControllerClose(pC);
            pCtx->pGamepads[i] = 0;
            pCtx->pGamepadIDs[i] = -1;
        }
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Bind a new controller
 * 
 * @param  pCtx  The event context
 * @param  index SDL index where the controller can be accessed
 * @param  pLog  The logging context
 */
static gfmRV gfmEvent_bindController(gfmEvent *pCtx, int index, gfmLog *pLog) {
    gfmRV rv;
    int i, num;
    
    // Sanitize arguments
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_debug, "Binding controllers...");
    ASSERT(rv == GFMRV_OK, rv);
    
    // Get how many controllers there are
    num = SDL_NumJoysticks();
    rv = gfmLog_log(pLog, gfmLog_debug, "  There are %i controllers connected",
            num);
    ASSERT(rv == GFMRV_OK, rv);
    
    if (pCtx->numControllers < num) {
        rv = gfmLog_log(pLog, gfmLog_debug, "  Expanding controllers array from"
                " %i to %i", pCtx->numControllers, num);
        ASSERT(rv == GFMRV_OK, rv);
        
        // Expand the game controllers' buffer
        pCtx->pGamepads = (SDL_GameController**)realloc(pCtx->pGamepads,
                num * sizeof(SDL_GameController**));
        ASSERT_LOG(pCtx->pGamepads, GFMRV_ALLOC_FAILED, pLog);
        pCtx->pGamepadIDs = (int*)realloc(pCtx->pGamepadIDs, num * sizeof(int));
        ASSERT_LOG(pCtx->pGamepadIDs, GFMRV_ALLOC_FAILED, pLog);
        // Clear the expanded part of the array
        memset(&(pCtx->pGamepads[pCtx->numControllers]), 0x00,
                sizeof(SDL_GameController**) * (num - pCtx->numControllers));
        memset(&(pCtx->pGamepadIDs[pCtx->numControllers]), 0xFF,
                sizeof(int) * (num - pCtx->numControllers));
        
        // Store the new length
        pCtx->numControllers = num;
    }
    
    // Try to bind the new controllers
    i = 0;
    while (i < num) {
        SDL_GameController *pC;
        
        // Find an empty slot
        pC = pCtx->pGamepads[i];
        if (pC == 0) {
            //char pGuidStr[33];
            int id;
            SDL_Joystick *pJ;
            
            // Try to open the controller
            pC = SDL_GameControllerOpen(index);
            // TODO If this fails, check if there's a mapping for the device's GUID,
            // load it and then try again
            ASSERT_LOG(pC, GFMRV_CONTROLLER_FAILED_TO_BIND, pLog);
            
            // Get its guid (so we can log)
            //guid = SDL_JoystickGetDeviceGUID(index);
            //SDL_JoystickGetGUIDString(guid, pGuidStr, 33);
            
            // Get its ID (so we can detect when its disconected)
            pJ = SDL_GameControllerGetJoystick(pC);
            ASSERT_LOG(pJ, GFMRV_INTERNAL_ERROR, pLog);
            id = (int)SDL_JoystickInstanceID(pJ);
            ASSERT_LOG(id >= 0, GFMRV_CONTROLLER_INVALID_ID, pLog);
            
            rv = gfmLog_log(pLog, gfmLog_debug, "  Bound Controller %i at index"
                    " %i", id, i);
            ASSERT(rv == GFMRV_OK, rv);
            //rv = gfmLog_log(pLog, gfmLog_debug, "  Bound Controller %*s to index %i",
            //        33, pGuidStr, id);
            //ASSERT(rv == GFMRV_OK, rv);
            
            // Store the controller and its id
            pCtx->pGamepads[i] = pC;
            pCtx->pGamepadIDs[i] = id;
            
            break;
        }
        
        i++;
    }
        
    rv = gfmLog_log(pLog, gfmLog_debug, "Done binding controllers!");
    ASSERT(rv == GFMRV_OK, rv);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Unbind a previously opened/bound controller
 * 
 * @param  pCtx The event context
 * @param  id   The gamepad id
 * @param  pLog The logging context
 */
static gfmRV gfmEvent_unbindController(gfmEvent *pCtx, int id, gfmLog *pLog) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_debug, "Unbinding controller...");
    ASSERT(rv == GFMRV_OK, rv);
    
    // Remove any disconnected controllers
    i = 0;
    while (i < pCtx->numControllers) {
        // Check if its the controller we are looking for
        if (pCtx->pGamepadIDs[i] == id) {
            SDL_GameController *pC;
            // And remove it!
            pC = pCtx->pGamepads[i];
            if (pC) {
                rv = gfmLog_log(pLog, gfmLog_debug, "  Removing controller with"
                        " ID %i from index %i", id, i);
                ASSERT(rv == GFMRV_OK, rv);
                
                // Yay! It doesn't return anything!
                SDL_GameControllerClose(pC);
                pCtx->pGamepads[i] = 0;
                pCtx->pGamepadIDs[i] = -1;
            }
            
            break;
        }
        i++;
    }
    
    rv = gfmLog_log(pLog, gfmLog_debug, "Done unbinding controller!");
    ASSERT(rv == GFMRV_OK, rv);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/


/**
 * Alloc a new event context
 * 
 * @param  ppCtx The event's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmEvent_getNew(gfmEvent **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the context
    *ppCtx = (gfmEvent*)malloc(sizeof(gfmEvent));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean the context
    memset(*ppCtx, 0x0, sizeof(gfmEvent));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated event
 * 
 * @param  ppCtx The event's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmEvent_free(gfmEvent **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    gfmEvent_clean(*ppCtx);
    // Free the context
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the event context
 * 
 * @param  pEvent The event's context
 * @param  pCtx   The event's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_init(gfmEvent *pEvent, gfmCtx *pCtx) {
    gfmLog *pLog;
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Retrieve the logger
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    // Continue to sanitize arguments
    ASSERT_LOG(pEvent, GFMRV_ARGUMENTS_BAD, pLog);
    
    // Initialize joystick
    irv = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    ASSERT_LOG(irv == 0, GFMRV_INTERNAL_ERROR, pLog);
    
    // Bind any connected joysticks
    rv = gfmEvent_bindAllControllers(pEvent, pLog);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    
    // Initialize the time event (to be pushed)
    pEvent->accTimerEvent.type = SDL_USEREVENT;
    pEvent->accTimerEvent.user.type = SDL_USEREVENT;
    pEvent->accTimerEvent.user.code = GFM_TIME_EVENT;
    pEvent->accTimerEvent.user.data1 = NULL;
    pEvent->accTimerEvent.user.data2 = NULL;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Remove any previously queued event
 * 
 * @param  pCtx The event's context
 */
gfmRV gfmEvent_clean(gfmEvent *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the last time to now
    pCtx->accLastTime = SDL_GetTicks();
    
    // Unbind any bound controllers
    gfmEvent_unbindAllControllers(pCtx);
    // Free its buffers
    if (pCtx->pGamepads) {
        free(pCtx->pGamepads);
    }
    if (pCtx->pGamepadIDs) {
        free(pCtx->pGamepadIDs);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Wait (i.e., block) until an event is available; The event should be stored
 * for later processing on gfmEvent_processQueued
 * 
 * @param  pCtx The event's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_waitEvent(gfmEvent *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Push a new time event */

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Process all queued events; This function MUSTN'T block
 * 
 * @param  pEv  The event's context
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_processQueued(gfmEvent *pEv, gfmCtx *pCtx) {
    gfmInput *pInput;
    gfmRV rv;
    SDL_Event ev;
    
    // Sanitize arguments
    ASSERT(pEv, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the input context
    rv = gfm_getInput(&pInput, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Poll every event
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_USEREVENT: {
                // User event is only used to update the timer
                switch (ev.user.code) {
                    case GFM_TIME_EVENT: {
                        unsigned int curTime, dt;
                        
                        // Update the event's timer
                        curTime = SDL_GetTicks();
                        dt = curTime - pEv->accLastTime;
                        pEv->accLastTime = curTime;
                        
                        // Update the timer on the game's context
                        if (dt > 0) {
                            rv = gfm_updateAccumulators(pCtx, dt);
                            ASSERT_NR(rv == GFMRV_OK);
                        }
                    } break;
                    default: {}
                }
            } break;
#if 0
            case SDL_FINGERMOTION: {
            } break;
            case SDL_FINGERDOWN: {
            } break;
            case SDL_FINGERUP: {
            } break;
#endif /* 0 */
			case SDL_MOUSEMOTION: {
                int x, y;
                
                // Get the position in the screen
                x = ev.motion.x;
                y = ev.motion.y;
                // Convert it to 'game space'
                rv = gfm_windowToBackbuffer(&x, &y, pCtx);
                ASSERT_NR(rv == GFMRV_OK);
                
                // Set the mouse position
                rv = gfmInput_setPointerPosition(pInput, x, y);
                ASSERT_NR(rv == GFMRV_OK);
            } break;
			case SDL_MOUSEBUTTONDOWN: {
                // Set mouse button as pressed
                rv = gfmInput_setKeyState(pInput, gfmPointer_button,
                        gfmInput_justPressed, ev.button.timestamp);
                ASSERT_NR(rv == GFMRV_OK);
            } break;
			case SDL_MOUSEBUTTONUP: {
                // Set mouse button as released
                rv = gfmInput_setKeyState(pInput, gfmPointer_button,
                        gfmInput_justReleased, ev.button.timestamp);
                ASSERT_NR(rv == GFMRV_OK);
            } break;
			case SDL_KEYDOWN: {
                gfmInputIface key;
                
                // Map SDL to gfmIface
                key = st_gfmEvent_convertSDLKey2GFM(ev.key.keysym.sym);
                // Set key as pressed
                rv = gfmInput_setKeyState(pInput, key, gfmInput_justPressed,
                        ev.key.timestamp);
                ASSERT_NR(rv == GFMRV_OK || key == gfmIface_none);
            } break;
			case SDL_KEYUP: {
                gfmInputIface key;
                
                // Map SDL to gfmIface
                key = st_gfmEvent_convertSDLKey2GFM(ev.key.keysym.sym);
                // Set key as released
                rv = gfmInput_setKeyState(pInput, key, gfmInput_justReleased,
                        ev.key.timestamp);
                ASSERT_NR(rv == GFMRV_OK || key == gfmIface_none);
            } break;
            case SDL_APP_WILLENTERBACKGROUND: {
                rv = gfm_pauseAudio(pCtx);
                ASSERT(rv == GFMRV_OK, rv);
            } break;
            case SDL_APP_WILLENTERFOREGROUND: {
                rv = gfm_resumeAudio(pCtx);
                ASSERT(rv == GFMRV_OK, rv);
            } break;
            case SDL_APP_TERMINATING:
            case SDL_QUIT: {
                // Signal to the main context that it should quit
                gfm_setQuitFlag(pCtx);
            } break;
            default: {}
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Push a time event; Should be called by gfmTimer
 * 
 * @param  pCtx The event's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_pushTimeEvent(gfmEvent *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Push the event
    SDL_PushEvent(&(pCtx->accTimerEvent));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

