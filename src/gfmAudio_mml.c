/**
 * @file src/include/GFraMe_int/gfmAudio_mml.h
 * 
 * Module to parse a MML audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe_int/gfmAudio_mml.h>

/**
 * Check if an audio file is encoded as mml
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isMml(gfmFile *pFp) {
    char pBuf[3];
    gfmRV rv;
    int count;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rv = gfmFile_rewind(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    // Try to read the first 3 bytes (that must contain {'M', 'M', 'L'}, to be a
    // valid mml file
    rv = gfmFile_readBytes(pBuf, &count, pFp, 3/*numBytes*/);
    ASSERT_NR(rv == GFMRV_OK);
    ASSERT(count == 3, GFMRV_READ_ERROR);
    // Check what was obtained
    ASSERT(pBuf[0] == 'M' && pBuf[1] == 'M' && pBuf[2] == 'L', GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

