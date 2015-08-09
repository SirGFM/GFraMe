/**
 * @file src/include/GFraMe_int/gfmAudio_wave.h
 * 
 * Module to parse a WAVE audio file
 */
#ifndef __GFMAUDIO_WAVE_H__
#define __GFMAUDIO_WAVE_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Check if an audio file is encoded as WAVE
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isWave(gfmFile *pFp);

/**
 * Loads a wave audio into a buffer
 * 
 * @param  ppBuf          Output buffer, must be cleared by the caller
 * @param  pLen           Size of the output buffer, in bytes
 * @param  pFp            The audio file
 * @param  freq           Audio sub-system's frequency/sampling rate
 * @param  bitsPerSamples Audio sub-system's bit per samples
 * @param  numChannels    Audio sub-system's number of channels
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR,
 *                        GFMRV_FUNCTION_FAILED, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 *                        GFMRV_ALLOC_FAILED
 */
gfmRV gfmAudio_loadWave(char **ppBuf, int *pLen, gfmFile *pFp, int freq,
        int bitsPerSample, int numChannels);

#endif /* __GFMAUDIO_WAVE_H__ */

