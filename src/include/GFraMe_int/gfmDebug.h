/**
 * @file src/include/GFraMe_int/gfmDebug.h
 *
 * Internal functions to initialize/clean the debug helpers.
 */
#ifndef __GFRAME_INT_GFMDEBUG_STRUCT__
#define __GFRAME_INT_GFMDEBUG_STRUCT__

/** Export the debug context */
typedef struct stGfmDebugCtx gfmDebugCtx;

#endif /* __GFRAME_INT_GFMDEBUG_STRUCT__ */

#ifndef __GFRAME_INT_GFMDEBUG_H__
#define __GFRAME_INT_GFMDEBUG_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/**
 * Initialize the debug context
 *
 * @param  [ in]pCtx The lib's main context
 */
gfmRV gfmDebug_init(gfmCtx *pCtx);

#endif /* __GFRAME_INT_GFMDEBUG_H__ */

