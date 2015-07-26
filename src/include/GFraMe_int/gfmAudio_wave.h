/**
 * @file src/include/GFraMe_int/gfmAudio_wave.h
 * 
 * Module to parse a WAVE audio file
 */
#ifndef __GFMAUDIO_WAVE_H__
#define __GFMAUDIO_WAVE_H__

#include <GFraMe/gfmError.h>

#include <stdio.h>

/**
 * Check if an audio file is encoded as WAVE
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isWave(FILE *pFp);

#endif /* __GFMAUDIO_WAVE_H__ */

