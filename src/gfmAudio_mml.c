/**
 * @file src/include/GFraMe_int/gfmAudio_mml.h
 * 
 * Module to parse a MML audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmAudio_mml.h>

#include <stdio.h>

/**
 * Check if an audio file is encoded as mml
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isMml(FILE *pFp) {
    char pBuf[3];
    gfmRV rv;
    int irv, count;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rewind(pFp);
    // Try to read the first 3 bytes (that must contain {'M', 'M', 'L'}, to be a
    // valid mml file
    count = 3;
    irv = fread(pBuf, sizeof(char), count, pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    // Check what was obtained
    ASSERT(pBuf[0] == 'M' && pBuf[1] == 'M' && pBuf[2] == 'L', GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

