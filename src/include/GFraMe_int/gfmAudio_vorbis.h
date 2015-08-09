/**
 * @file src/include/GFraMe_int/gfmAudio_vorbis.h
 * 
 * Module to parse a vorbis audio file
 */
#ifndef __GFMAUDIO_VORBIS_H__
#define __GFMAUDIO_VORBIS_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Check if an audio file is encoded as vorbis
 * 
 * @param  pFp The file pointer
 * @return     GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmAudio_isVorbis(gfmFile *pFp);

#endif /* __GFMAUDIO_VORBIS_H__ */

