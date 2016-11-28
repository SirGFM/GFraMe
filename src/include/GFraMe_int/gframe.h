/**
 * @file src/include/GFraMe_int/gframe.h
 *
 * Main module internal functions.
 */
#ifndef __GFRAME_INT_GFRAME_H__
#define __GFRAME_INT_GFRAME_H__

/**
 * Create and load a texture; the lib will keep track of it and release its
 * memory, on exit
 * 
 * @param  pIndex   The texture's index
 * @param  pCtx     The game's contex
 * @param  pData    The texture's data (32 bits, [0xRR, 0xGG, 0xBB, 0xAA,...])
 * @param  width    The texture's width
 * @param  height   The texture's height
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_BITMAP,
 *                  GFMRV_TEXTURE_FILE_NOT_FOUND,
 *                  GFMRV_TEXTURE_INVALID_WIDTH,
 *                  GFMRV_TEXTURE_INVALID_HEIGHT, GFMRV_ALLOC_FAILED,
 *                  GFMRV_INTERNAL_ERROR
 */
gfmRV _gfm_loadBinTexture(int *pIndex, gfmCtx *pCtx, char *pData, int width,
        int height);

#endif /* __GFRAME_INT_GFRAME_H__ */

