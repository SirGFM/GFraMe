/**
 * @file src/include/GFraMe_int/gfmVideo_bmp.h
 *
 * Loads a 24-bits bitmap file into a buffer
 */
#ifndef __GFMVIDEO_BMP_H__
#define __GFMVIDEO_BMP_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Check if a given file is a bitmap
 *
 * @param  [ in]pFp  The opened file
 * @param  [ in]pLog The logger
 * @return           GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmVideo_isBmp(gfmFile *pFp, gfmLog *pLog);

/**
 * Loads a file into a buffer (in 24-bits 0xRRGGBB format)
 *
 * NOTE: The buffer is callee alloc'ed, so it must be freed by the caller!
 *
 * @param  [out]pBuf     The loaded image
 * @param  [out]pWidth   The image's width
 * @param  [out]pHeight  The image's height
 * @param  [ in]pFp      The image's file
 * @param  [ in]pLog     The logger
 * @param  [ in]colorKey 24 bits, RGB Color to be treated as transparent
 * @return               GFMRV_OK, 
 */
gfmRV gfmVideo_loadFileAsBmp(char **pBuf, int *pWidth, int *pHeight,
        gfmFile *pFp, gfmLog *pLog, int colorKey);

#endif /* __GFMVIDEO_BMP_H__ */

