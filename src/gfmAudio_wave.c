/**
 * @file src/include/GFraMe_int/gfmAudio_wave.h
 * 
 * Module to parse a WAVE audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmAudio_wave.h>

#include <stdio.h>

/**
 * Check if an audio file is encoded as WAVE
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isWave(FILE *pFp) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rewind(pFp);
    
    // TODO
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

