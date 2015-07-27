/**
 * @file src/include/GFraMe_int/gfmAudio_vorbis.h
 * 
 * Module to parse a vorbis audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmAudio_vorbis.h>

#include <stdio.h>

/**
 * Check if an audio file is encoded as vorbis
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isVorbis(FILE *pFp) {
    char pBuf[7];
    gfmRV rv;
    int irv, count;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    // Rewind the file
    rewind(pFp);
    // Try to read the first 7 bytes (that must contain {<type>, 'v', 'o', 'r'
    //  'b', 'i', 's'}, to be a valid vorbis file
    count = 7;
    irv = fread(pBuf, sizeof(char), count, pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    // Check what was obtained
    ASSERT(pBuf[1] == 'v' && pBuf[2] == 'o' && pBuf[3] == 'r' && pBuf[4] == 'b'
            && pBuf[5] == 'i' && pBuf[6] == 's', GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

