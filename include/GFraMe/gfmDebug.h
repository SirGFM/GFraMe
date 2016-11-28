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

#include <GFraMe/gframe.h>

/**
 * Print a string to the screen.
 *
 * @param  [ in]pCtx  The framework's main context
 * @param  [ in]x     The horizontal position in screen space
 * @param  [ in]y     The vertical position in screen space
 * @param  [ in]pText The text to be printed
 * @param  [ in]len   The length of the text
 */
void gfmDebug_printText(gfmCtx *pCtx, int x, int y, char *pText, int len);

/**
 * Print an integer number in decimal base to the screen.
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]val  The number to be printed
 * @param  [ in]len  Minimum number of digits (padded with 0s)
 */
void gfmDebug_printInt(gfmCtx *pCtx, int x, int y, int val, int len);

/**
 * Print an integer number in hexadecimal base number to the screen.
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]hexa The number to be printed
 * @param  [ in]len  Minimum number of digits (padded with 0s)
 */
void gfmDebug_printHexa(gfmCtx *pCtx, int x, int y, int hexa, int len);

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

#endif /* __GFMDEBUG_H__ */

