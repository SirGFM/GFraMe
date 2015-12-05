/**
 * @file src/include/GFraMe_int/gfmAudio_mml.h
 * 
 * Module to parse a MML audio file
 */
#ifndef __GFMAUDIO_MML_H__
#define __GFMAUDIO_MML_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

#include <c_synth/synth.h>
#include <c_synth/synth_errors.h>

/**
 * Structure to hold a MML track;
 * 
 * MML songs are parsed into at least one of these tracks; Each track plays
 * independently, though they are all controlled by the same song(to manipulate
 * its volume and position);
 * Tracks are sub-divided into 'chunks': samples that plays contiguously; Those
 * 'chunks' have its order of playing mapped by an array, so a repeatition is
 * actually done by mapping the 'chunk' twice, instead of rendering it twice
 * (and spending double the required memory);
 */
struct stMmlTrack {
    char **ppChunks;
    int *pChunkLen;
    int numChunks;
    int *pChunkMap;
    int mapLen;
    int initChunk;
};
typedef struct stMmlTrack mmlTrack;

/**
 * Check if an audio file is encoded as mml
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isMml(gfmFile *pFp);

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
        gfmLog *pLog, synthCtx *pCtx, synthBufMode mode);

#endif /* __GFMAUDIO_MML_H__ */

