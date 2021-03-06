/**
 * @file include/GFraMe/core/gfmAudio_bkend.h
 * 
 * Backend to load and play audios; Audio playback is expect to run
 * asynchronously, so it's the backend responsability to handle when its
 * processing thread should awake/sleep;
 * Before playing/loading any sound, it's necessary to manually initialize this
 * subsystem; During this initialization, one can define the system's quality;
 * At highers qualities, the system must respond more quickly and have more bits
 * per samples; TODO Check if this will be really done...
 * Then, audio files may be loaded into "audio structs"; Those should be managed
 * by the backend itself, only returning a handle (i.e., int) to the user; Each
 * audio may be set as repeating, with a custom repeat point (i.e., start at
 * position 0 but, when looping, goes back to position X);
 * When an audio is requested to be played, it must return a gfmAudioHandle;
 * This structure represents a instance of the played audio and must be used to
 * enabled modifing its volume, as way as making it stop playing;
 */
#ifndef __GFMAUDIO_BKEND_STRUCT__
#define __GFMAUDIO_BKEND_STRUCT__

/** Audio sub-system context */
typedef struct stGFMAudioCtx gfmAudioCtx;
/** Custom audio handle; Is returned when an audio is played */
typedef struct stGFMAudioHandle gfmAudioHandle;

#endif /* __GFMAUDIO_BKEND_STRUCT__ */

#ifndef __GFMAUDIO_BKEND_H__
#define __GFMAUDIO_BKEND_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gframe.h>

/** Custom quality settings */
enum enGFMAudioQuality {
    /** Possibles number of channels */
    gfmAudio_stereo      = 0x000000,   /* A single channel */
    gfmAudio_mono        = 0x000001,   /* Two channels     */
  //gfmAudio_5           = 0x000002,   /* Five channels    */
    /** Possible sample rates        */
    gfmAudio_defFreq     = 0x000000,   /* 44100 Hertz      */
    gfmAudio_lowFreq     = 0x000010,   /* 11025 Hertz      */
    gfmAudio_medFreq     = 0x000020,   /* 22050 Hertz      */
    gfmAudio_highFreq    = 0x000040,   /* 88200 Hertz      */
    /** Default settings             */
    gfmAudio_defQuality  = gfmAudio_stereo | gfmAudio_defFreq,
    gfmAudio_lowQuality  = gfmAudio_mono   | gfmAudio_lowFreq,
    gfmAudio_medQuality  = gfmAudio_stereo | gfmAudio_medFreq,
    gfmAudio_highQuality = gfmAudio_stereo | gfmAudio_highFreq
  //gfmAudio_highQuality = gfmAudio_5      | gfmAudio_highFreq
};
typedef enum enGFMAudioQuality gfmAudioQuality;

/**
 * Alloc a new gfmAudioCtx
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAudio_getNew(gfmAudioCtx **ppCtx);

/**
 * Frees (and close, if it was initialized) the audio context
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAudio_free(gfmAudioCtx **ppCtx);

/**
 * Initialize the audio subsystem
 * 
 * @param  pAudio  The audio context
 * @param  pCtx    The game's context
 * @param  setting The desired audio settings
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_AUDIO_ALREADY_INITIALIZED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmAudio_initSubsystem(gfmAudioCtx *pAudio, gfmCtx *pCtx,
        gfmAudioQuality settings);

/**
 * Clear all alloc'ed memory and closed the subsystem
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD
 */
gfmRV gfmAudio_closeSubSystem(gfmAudioCtx *pCtx);

/**
 * Stop the audio system, if it was playing
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_resumeSubsystem(gfmAudioCtx *pCtx);

/**
 * Pauses the audio system; It will restart playing as soon as a new audio is
 * played or gfmAudio_resumeSubSystem is called
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_pauseSubsystem(gfmAudioCtx *pCtx);

/**
 * Loads an audio from a file, converting it (if possible) to the current
 * format and sample rate; The file must be in the assets folder, but only the
 * relative path to assets should be sent
 * 
 * @param  pHandle     The audio handle (used to play it)
 * @param  pAud        The audio context
 * @param  pCtx        The game's context
 * @param  pFilename   The audio filename
 * @param  filenameLen The filename length
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_COULDNT_OPEN_FILE,
 *                     GFMRV_READ_ERROR, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 *                     GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmAudio_loadAudio(int *pHandle, gfmAudioCtx *pAud, gfmCtx *pCtx,
        char *pFilename, int filenameLen);

/**
 * Set to which sample the song must loop
 * 
 * @param  pCtx   The audio context
 * @param  handle The handle of the looped audio
 * @param  pos    Sample to which the song should go back when looping
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_INVALID_BUFFER_LEN, GFMRV_AUDIO_INVALID_TYPE
 */
gfmRV gfmAudio_setRepeat(gfmAudioCtx *pCtx, int handle, int pos);

/**
 * Queue an audio and return its instance'a=s handle (so you can pause/play/stop
 * it and change its volume)
 *
 * NOTE: This function doesn't automatically resume the audio subsystem
 * 
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx   The audio context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfmAudio_queueAudio(gfmAudioHandle **ppHnd, gfmAudioCtx *pCtx, int handle,
        double volume);

/**
 * Play an audio and return its instance's handle (so you can pause/play/stop it
 * and change its volume)
 * 
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx   The audio context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfmAudio_playAudio(gfmAudioHandle **ppHnd, gfmAudioCtx *pCtx, int handle,
        double volume);

/**
 * Stops an audio instance
 * 
 * @param  pCtx  The audio context
 * @param  ppHnd The instance to be stopped
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_stopAudio(gfmAudioCtx *pCtx, gfmAudioHandle **ppHnd);

/**
 * Pause a currently playing audio
 * 
 * @param  pCtx The audio context
 * @param  pHnd The instance handle
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_pauseAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd);

/**
 * Resume a paused audio
 * 
 * @param  pCtx The audio context
 * @param  pHnd The instance handle
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_resumeAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd);

/**
 * Set an audio's volume
 * 
 * @param  pCtx   The audio context
 * @param  pHnd   The instance handle
 * @param  volume The new volume (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_setHandleVolume(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd,
        double volume);

/**
 * Check whether an audio handle finished played
 * 
 * @param  pCtx The audio context
 * @param  pHnd The instance handle
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAudio_didHandleFinish(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd);

/**
 * Whether any track may have multiple tracks; Each track should have its own
 * loop position and volume
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmAudio_isTrackSupported(gfmAudioCtx *pCtx);

gfmRV gfmAudio_getNumTracks(int *pNum, gfmAudioCtx *pCtx, int handle);
gfmRV gfmAudio_setTrackVolume(gfmAudioCtx *pCtx, int handle, double volume);

#endif /* __GFMAUDIO_BKEND_H__ */

