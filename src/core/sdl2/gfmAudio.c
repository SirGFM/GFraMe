/**
 * @file src/core/sdl2/gfmAudio.c
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
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/core/gfmAudio_bkend.h>

// Required because of SDL_InitSubSystem... >_<
#include <SDL2/SDL.h>

#include <stdlib.h>
#include <string.h>

/** An audio representation */
typedef struct stGFMAudio gfmAudio;

/** Define an array of nodes */
gfmGenArr_define(gfmAudioHandle);
/** Define an array of audios */
gfmGenArr_define(gfmAudio);

/** Audio sub-system context */
struct stGFMAudioCtx {
    /** Pool of audios */
    gfmGenArr_var(gfmAudio, pAudioPool);
    /** Pool of audio handles */
    gfmGenArr_var(gfmAudioHandle, pAudioHndPool);
    /** List of available audio handles (for recycling) */
    gfmAudioHandle *pAudioHndAvailable;
    /** List of playing audio handles */
    gfmAudioHandle *pAudioHndPlaying;
    /** RV from the last time the callback was executed */
    gfmRV callbackRV;
    /** Whether the subsystem was initialized */
    int init;
    /** Whether the subsystem is playing or was paused */
    int isPlaying;
    /** ID of the opened audio device */
    SDL_AudioDeviceID dev;
    /** Specs of the opened audio device */
    SDL_AudioSpec spec;
};

/** An audio representation */
struct stGFMAudio {
    /** Audio samples buffer */
    unsigned char *pBuf;
    /** Number of bytes in pBuf */
    int len;
    /** Whether should repeat */
    int doRepeat;
    /** Position to which should jump to, on repeat */
    int repeatPosition;
};

/** Custom audio handle; Is returned when an audio is played */
struct stGFMAudioHandle {
    /** Audio played by this node */
    gfmAudio *pSelf;
    /** Next audio being played/available for recycling */
    gfmAudioHandle *pNext;
    /** Volume at which this should be played */
    int volume;
    /** Position in the current audio */
    int pos;
};

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Callback called by SDL2 to fill the audio buffer
 * 
 * @param  pArg    The audio context (set on SDL_OpenAudioDevice)
 * @param  pStream The buffer to be filled
 * @param  len     How many bytes there are in the buffer
 */
static void gfmAudio_callback(void *pArg, Uint8 *pStream, int len) {
    gfmAudioCtx *pCtx;
    gfmAudioHandle *pTmp;
    gfmRV rv;
    int isLocked;
    
    // Set default values
    isLocked = 0;
    
    // Sanitize arguments
    ASSERT(pArg, GFMRV_ARGUMENTS_BAD);
    ASSERT(pStream, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the audio context
    pCtx = (gfmAudioCtx*)pArg;
    
    // TODO Lock the mutex
    isLocked = 1;
    
    // Clean the output buffer (so there's no noise)
    memset(pStream, 0x0, len);
    
    // Loop through every node
    pTmp = pCtx->pAudioHndPlaying;
    while (pTmp) {
        // TODO Play the buffer
        // Go to the next node
        pTmp = pTmp->pNext;
    }
    // TODO Pause the device if there're no more audios
__ret:
    if (isLocked) {
        // Set the return value on the context
        pCtx->callbackRV = rv;
        // TODO Unlock the mutex
    }
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Alloc a new gfmAudioCtx
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAudio_getNew(gfmAudioCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the struct
    *ppCtx = (gfmAudioCtx*)malloc(sizeof(gfmAudioCtx));
    ASSERT(ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmAudioCtx));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees (and close, if it was initialized) the audio context
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAudio_Free(gfmAudioCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // If it was initialized, close the subsystem
    if ((*ppCtx)->init) {
        gfmAudio_closeSubSystem(*ppCtx);
    }
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the audio subsystem
 * 
 * @param  pCtx    The audio context
 * @param  setting The desired audio settings
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_AUDIO_ALREADY_INITIALIZED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmAudio_initSubsystem(gfmAudioCtx *pCtx, gfmAudioQuality settings) {
    gfmRV rv;
    int irv;
    SDL_AudioSpec wanted;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it still wasn't initialized
    ASSERT(!pCtx->init, GFMRV_AUDIO_ALREADY_INITIALIZED);
    // TODO Add support for others settings
    ASSERT(settings == gfmAudio_defQuality, GFMRV_FUNCTION_NOT_SUPPORTED);
    
    // Initialize the audio subsystem
    irv = SDL_InitSubSystem(SDL_INIT_AUDIO);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Set the system as initialized
    pCtx->init = 1;
    
    // Set the desired audio format
    // Samples per second
    wanted.freq = 44100;
    // Sample format (i.e., signedness, endianess, number of bits per samples)
    wanted.format = AUDIO_S16LSB;
    // Number of channels (defaults to stereo)
    wanted.channels = 2;
    // Callback used to fill the buffer
    wanted.callback = gfmAudio_callback;
    // Send the audio context itself to the callback
    wanted.userdata = (void*)pCtx;
    
    // Open the audio device with the requested format (NOTE: SDL2 already
    // starts the device paused!!)
    pCtx->dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &(pCtx->spec), 0);
    ASSERT(pCtx->dev > 0, GFMRV_INTERNAL_ERROR);
    
    // TODO Initialize mutexes
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmAudio_closeSubSystem(gfmAudioCtx *pCtx);

/**
 * Stop the audio system, if it was playing
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_resumeSubsystem(gfmAudioCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    // Check that it was playing
    ASSERT(pCtx->isPlaying, GFMRV_OK);
    
    // TODO Lock the mutex
    // Only unpause if there's anything to play
    if (pCtx->pAudioHndPlaying) {
        SDL_PauseAudioDevice(pCtx->dev, 1);
    }
    // TODO Unlock the mutex
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pauses the audio system; It will restart playing as soon as a new audio is
 * played or gfmAudio_resumeSubSystem is called
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_pauseSubsystem(gfmAudioCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    // Check that it was paused
    ASSERT(!(pCtx->isPlaying), GFMRV_OK);
    
    // TODO Lock the mutex
    SDL_PauseAudioDevice(pCtx->dev, 0);
    // TODO Unlock the mutex
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmAudio_loadAudio(int *pHandle, gfmAudioCtx *pCtx, char *pFilename,
        int filenameLen);
gfmRV gfmAudio_setRepeat(gfmAudioCtx *pCtx, int handle, int pos);

gfmRV gfmAudio_playAudio(gfmAudioHandle **ppHnd, gfmAudioCtx *pCtx, int handle
        , int volume);
gfmRV gfmAudio_stopAudio(gfmAudioCtx *pCtx, gfmAudioHandle **ppHnd);
gfmRV gfmAudio_pauseAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd);
gfmRV gfmAudio_resumeAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd);
gfmRV gfmAudio_setHandleVolume(gfmAudioHandle *pCtx, int volume);

gfmRV gfmAudio_isTrackSupported(gfmAudioCtx *pCtx);
gfmRV gfmAudio_getNumTracks(int *pNum, gfmAudioCtx *pCtx, int handle);
gfmRV gfmAudio_setTrackVolume(gfmAudioCtx *pCtx, int handle, int volume);

