/**
 * @file src/include/GFraMe_int/gfmDebug.h
 *
 * Internal functions to initialize/clean the debug helpers.
 */
#ifndef __GFRAME_INT_GFMDEBUG_H__
#define __GFRAME_INT_GFMDEBUG_H__

#if defined(DEBUG)

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/**
 * Initialize the debug context
 *
 * @param  [ in]pCtx The lib's main context
 */
gfmRV gfmDebug_init(gfmCtx *pCtx);

#else
#define gfmDebug_init(...) GFMRV_OK
#endif /* defined(DEBUG) */


#endif /* __GFRAME_INT_GFMDEBUG_H__ */

