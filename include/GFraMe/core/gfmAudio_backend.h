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

gfmRV gfmAudio_initSubsystem(gfmAudioCtx **ppCtx);
gfmRV gfmAudio_closeSubSystem(gfmAudioCtx **ppCtx);
gfmRV gfmAudio_resumeSubsystem(gfmAudioCtx *pCtx);
gfmRV gfmAudio_pauseSubsystem(gfmAudioCtx *pCtx);

gfmRV gfmAudio_loadAudio(int *pHandle, gfmAudioCtx *pCtx, char *pFilename,
        int filenameLen);
gfmRV gfmAudio_setRepeat(gfmAudioCtx *pCtx, int handle, int pos);

gfmRV gfmAudio_playAudio(gfmAudioHandle **ppCtx, gfmAudio *pCtx, int handle
        , int volume);
gfmRV gfmAudio_stopAudio(gfmAudioHandle **ppCtx, gfmAudio *pCtx);
gfmRV gfmAudio_pauseAudio(gfmAudioHandle *pCtx, gfmAudio *pCtx);
gfmRV gfmAudio_resumeAudio(gfmAudioHandle *pCtx, gfmAudio *pCtx);
gfmRV gfmAudio_setHandleVolume(gfmAudioHandle *pCtx, int volume);

#endif /* __GFMAUDIO_BKEND_H__ */

