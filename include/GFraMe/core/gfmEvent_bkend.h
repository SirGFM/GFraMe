/**
 * @file include/GFraMe/core/gfmEvent_bkend.h
 * 
 * Module for managing events (as input, timer, resize, etc)
 */
#ifndef __GFMEVENT_STRUCT__
#define __GFMEVENT_STRUCT__

/** 'Exports' the gfmEvent structure */
typedef struct stGFMEvent gfmEvent;

#endif /* __GFMEVENT_STRUCT__ */

#ifndef __GFMEVENT_BKEND_H__
#define __GFMEVENT_BKEND_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/** 'Exportable' size of gfmEvent */
extern const int sizeofGFMEvent;

/**
 * Alloc a new event context
 * 
 * @param  ppCtx The event's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmEvent_getNew(gfmEvent **ppCtx);

/**
 * Free a previously allocated event
 * 
 * @param  ppCtx The event's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmEvent_free(gfmEvent **ppCtx);

/**
 * Remove any previously queued event
 * 
 * @param  pCtx The event's context
 */
gfmRV gfmEvent_clean(gfmEvent *pCtx);

/**
 * Wait (i.e., block) until an event is available; The event should be stored
 * for later processing on gfmEvent_processQueued
 * 
 * @param  pCtx The event's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_waitEvent(gfmEvent *pCtx);

/**
 * Process all queued events; This function MUSTN'T block
 * 
 * @param  pEv  The event's context
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_processQueued(gfmEvent *pEv, gfmCtx *pCtx);

/**
 * Push a time event; Should be called by gfmTimer
 * 
 * @param  pCtx   The event's context
 * @param  pEvent Backend's representation for the event
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmEvent_pushTimeEvent(gfmEvent *pCtx, void *pEvent);

// TODO interface to manually handle events

#endif /* __GFMEVENT_BKEND_H__ */

