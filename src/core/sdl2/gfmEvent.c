/**
 * @file src/core/sdl2/gfmEvent.h
 * 
 * Module for managing events (as input, timer, resize, etc)
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmEvent_bkend.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include <stdlib.h>
#include <string.h>

/** The gfmEvent structure */
struct stGFMEvent {
    /** The last time accumulated */
    unsigned int lastTime;
    /** Event that will be pushed on every timer callback */
    SDL_Event timeEvent;
};

/** Size of gfmEvent */
const int sizeofGFMEvent = (int)sizeof(gfmEvent);

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
    
    // Initialize the time event (to be pushed)
    (*ppCtx)->timeEvent.type = SDL_USEREVENT;
    (*ppCtx)->timeEvent.user.type = SDL_USEREVENT;
    (*ppCtx)->timeEvent.user.code = GFM_TIME_EVENT;
    (*ppCtx)->timeEvent.user.data1 = NULL;
    (*ppCtx)->timeEvent.user.data2 = NULL;
    
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
    
    // Free the context
    free(*ppCtx);
    *ppCtx = 0;
    
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
    pCtx->lastTime = SDL_GetTicks();
    
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
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    irv = SDL_WaitEvent(0);
    ASSERT(irv == 1, GFMRV_INTERNAL_ERROR);
    
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
    gfmRV rv;
    SDL_Event ev;
    
    // Sanitize arguments
    ASSERT(pEv, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
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
                        dt = curTime - pEv->lastTime;
                        pEv->lastTime = curTime;
                        
                        // Update the timer on the game's context
                        if (dt > 0) {
                            rv = gfm_updateAccumulators(pCtx, dt);
                            ASSERT_NR(rv == GFMRV_OK);
                        }
                    } break;
                    default: {}
                }
            } break;
            case SDL_QUIT: {
                // Signal to the main context that it should quit
                gfm_setQuitFlag(pCtx);
            } break;
            // TODO ....
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
    SDL_PushEvent(&(pCtx->timeEvent));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

