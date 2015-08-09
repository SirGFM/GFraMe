/**
 * @file src/include/GFraMe_int/gfmAudio_mml.h
 * 
 * Module to parse a MML audio file
 */
#ifndef __GFMAUDIO_MML_H__
#define __GFMAUDIO_MML_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Check if an audio file is encoded as mml
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isMml(gfmFile *pFp);

#endif /* __GFMAUDIO_MML_H__ */

