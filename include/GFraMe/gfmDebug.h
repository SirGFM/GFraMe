/**
 * @file include/GFraMe/gfmDebug.h
 *
 * Utilities to help debug the game.
 *
 * Contrary to the rest of the lib, this file is controlled by a 'DEBUG' flag.
 * If not set, the header gets disabled in a way that avoids breaking the
 * compilation.
 */
#ifndef __GFMDEBUG_H__
#define __GFMDEBUG_H__

#if defined(DEBUG)

#include <GFraMe/gframe.h>

/**
 * Print a format string (similar to printf)
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]pFmt The string's format (similar to printf's)
 * @param  [ in]...  The string's arguments (similar to printf's)
 */
void gfmDebug_printf(gfmCtx *pCtx, int x, int y, const char *pFmt, ...);

#else
#define gfmDebug_printf(...) do { } while(0)
#endif /* defined(DEBUG) */

#endif /* __GFMDEBUG_H__ */

