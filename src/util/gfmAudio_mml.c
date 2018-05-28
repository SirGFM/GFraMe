/**
 * @file src/util/gfmAudio_mml.c
 * 
 * Module to parse a MML audio file
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe_int/gfmAudio_mml.h>

#include <stdlib.h>
#include <string.h>

#include <c_synth/synth.h>
#include <c_synth/synth_errors.h>

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
    ASSERT((pBuf[0] == 'M' && pBuf[1] == 'M' && pBuf[2] == 'L') ||
            (pBuf[0] == 'm' && pBuf[1] == 'm' && pBuf[2] == 'l'), GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Loads a MML song merging all the tracks into a single buffer
 * 
 * @param  [out]ppBuf Output buffer, must be cleared by the caller
 * @param  [out]pLen  Size of the output buffer, in bytes
 * @param  [out]pLoop Song's loop position, in bytes
 * @param  [ in]pFp   The audio file
 * @param  [ in]pCtx  Synthesizer context
 * @param  [ in]mode  Mode (i.e., number of channels and bits per samples)
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR,
 *                    GFMRV_FUNCTION_FAILED, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 *                    GFMRV_ALLOC_FAILED
 */
gfmRV gfmAudio_loadMMLAsWave(char **ppBuf, int *pLen, int *pLoop, gfmFile *pFp,
        gfmLog *pLog, synthCtx *pCtx, synthBufMode mode) {
    char *pBuf, *pDst;
#if !defined(GFRAME_MOBILE)
    char *pFilename;
#else
    void *pFile;
#endif
    gfmRV rv;
    int bufLen, doLoop, handle, loopPos, numBytes;
    synth_err srv;
    
    // Set default values
    pBuf = 0;
    pDst = 0;
    
    // Sanitize arguments
    ASSERT_LOG(pFp, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppBuf, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(!(*ppBuf), GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pLen, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pLoop, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    
    // Retrieve the song filename
#if !defined(GFRAME_MOBILE)
    rv = gfmFile_getPath(&pFilename, pFp);
    ASSERT_NR(rv == GFMRV_OK);
#else
    rv = gfmFile_rewind(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmFile_getInternalObject(&pFile, pFp);
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // Retrieve the number of bytes
    numBytes = 1;
    if (mode & SYNTH_2CHAN) {
        numBytes *= 2;
    }
    if (mode & SYNTH_16BITS) {
        numBytes *= 2;
    }
    
    // Compile the song
#if !defined(GFRAME_MOBILE)
    srv = synth_compileSongFromFile(&handle, pCtx, pFilename);
#else
    srv = synth_compileSongFromSDL_RWops(&handle, pCtx, pFile);
#endif
    ASSERT(srv == SYNTH_OK, GFMRV_INTERNAL_ERROR);
    
    // Get the number of samples in the song
    srv = synth_getSongLength(&bufLen, pCtx, handle);
    ASSERT(srv == SYNTH_OK, GFMRV_INTERNAL_ERROR);
    
    // Alloc the final buffer for the song
    pDst = (char*)malloc(bufLen * numBytes * sizeof(char));
    ASSERT(pDst, GFMRV_ALLOC_FAILED);
    
    // Alloc a temporary buffer, used to render each track
    pBuf = (char*)malloc(bufLen * numBytes * sizeof(char));
    ASSERT(pBuf, GFMRV_ALLOC_FAILED);
    
    // Render the song
    srv = synth_renderSong(pDst, pCtx, handle, mode, pBuf);
    ASSERT(srv == SYNTH_OK, GFMRV_INTERNAL_ERROR);
    
    // Check if it actually loops
    srv = synth_canSongLoop(pCtx, handle);
    ASSERT(srv == SYNTH_OK || srv == SYNTH_NOT_LOOPABLE, GFMRV_INTERNAL_ERROR);
    if (srv == SYNTH_OK) {
        // Retrieve the loop position (in samples)
        srv = synth_getSongIntroLength(&loopPos, pCtx, handle);
        ASSERT(srv == SYNTH_OK, GFMRV_INTERNAL_ERROR);
        
        doLoop = 1;
    }
    else {
        loopPos = 0;
        doLoop = 0;
    }
    
    // Set all returns
    *ppBuf = pDst;
    *pLen = bufLen * numBytes * sizeof(char);
    if (doLoop) {
        *pLoop = loopPos * numBytes * sizeof(char);
    }
    else {
        *pLoop = -1;
    }
    
    rv = GFMRV_OK;
__ret:
    if (pBuf) {
        free(pBuf);
    }
    if (rv != GFMRV_OK && pDst) {
        free(pDst);
    }
    
    return rv;
}

#if 0
/**
 * Loads a mml file; Each of its track will be put into its own separate buffer,
 * which should be looped and played separately
 */
gfmRV gfmAudio_loadMml(char ***pppBufs, int **ppLens, int **ppLoopPoints,
        int *pNum, gfmFile *pFp, gfmLog *pLog, int freq, int bitsPerSample,
        int numChannels) {
    char **ppBufs;
    gfmRV rv;
    int bpm, num, *pLens, *pLoopPoints;
    
    // Set everything to zero so it can be cleaned on error
    ppBufs = 0;
    pLens = 0;
    pLoopPoints = 0;
    num = 0;
    
    // Sanitize arguments
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pFp, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pppBufs, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG((*pppBufs) == 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppLens, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG((*ppLens) == 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppLoopPoints, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG((*ppLoopPoints) == 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pNum, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(freq > 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(bitsPerSample == 8 || bitsPerSample == 16, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(numChannels > 0, GFMRV_ARGUMENTS_BAD, pLog);
    
    // Rewind the file
    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    
    // TODO Parse 'MML' header and get bpm
    // TODO Parse each track
    
    // Set the return
    *pppBufs = ppBufs;
    *ppLens = pLens;
    *ppLoopPoints = pLoopPoints;
    *pNum = num;
    rv = GFMRV_OK;
    // Set these to NULL so they aren't mistakenly free'd
    ppBufs = 0;
    pLens = 0;
    pLoopPoints = 0;
__ret:
    // Free the buffers, on error
    if (ppBufs) {
        int i;
        
        i = 0;
        while (i < num) {
            free(ppBufs[i]);
            i++;
        }
        free(ppBufs);
    }
    ppBufs = 0;
    if (pLens ) {
        free(pLens );
    }
    pLens = 0;
    if (pLoopPoints ) {
        free(pLoopPoints );
    }
    pLoopPoints = 0;
    
    return rv;
}
#endif /* 0 */

