/**
 * @file src/core/loadAsync/gfmLoadAsync_SDL2.c
 *
 * Load assets asynchronously. This implementation uses SDL2 for threading.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gframe.h>

#include <GFraMe_int/core/gfmLoadAsync_bkend.h>

#include <SDL2/SDL_thread.h>

#include <stdlib.h>
#include <string.h>

struct stGFMLoadAsyncCtx {
    /** Log's context */
    gfmLog *pLog;
    /** The thread handle */
    SDL_Thread *pThread;
    /** The framework's main context */
    gfmCtx *pCtx;
    /** Types of assets to be loaded */
    gfmAssetType *pType;
    /** Path to assets */
    char **ppPath;
    /** List of pointer where handles shall be stored */
    int **ppHandles;
    /** Update loading progress (in number of assets loaded) */
    int *pProgress;
    /** Number of assets to be loaded */
    int numAssets;
    /** Thread's return value (usefull in case of error) */
    gfmRV rv;
};

/**
 * Thread for loading every asset
 *
 * @param  [ in]pCtx Argument passed at thread generation
 */
static int _gfmLoadAsync_thread(void *pCtx) {
    /** The async loader */
    gfmLoadAsyncCtx *pLoader;
    /** GFraMe return value */
    gfmRV rv;
    /** Iterator for the assets */
    int i;

    /* Cast the received context to the loader */
    pLoader = (gfmLoadAsyncCtx*)pCtx;
    ASSERT(pLoader, GFMRV_INTERNAL_ERROR);

    i = 0;
    while (i < pLoader->numAssets) {
        switch (pLoader->pType[i]) {
            case ASSET_TEXTURE: {
                /* Load a texture */
                rv = gfm_loadTexture(pLoader->ppHandles[i], pLoader->pCtx,
                        pLoader->ppPath[i], strlen(pLoader->ppPath[i]),
                        0xff00ff);
            } break;
            case ASSET_AUDIO: {
                /* Load an audio */
                rv = gfm_loadAudio(pLoader->ppHandles[i], pLoader->pCtx,
                        pLoader->ppPath[i], strlen(pLoader->ppPath[i]));
            } break;
            default: {
                ASSERT_LOG(0, GFMRV_ASYNC_LOADER_INVALID_TYPE,
                        pLoader->pLog);
            }
            ASSERT_LOG(rv == GFMRV_OK, rv, pLoader->pLog);
        }

        i++;
        *(pLoader->pProgress) = i;
    }

    rv = GFMRV_OK;
__ret:
    /* Store the return value */
    if (pLoader) {
        pLoader->rv = rv;
    }

    return rv;
}

/**
 * Finishes the loader thread and release all memory
 *
 * @param  [ in]ppLoader  The asynchronous loader context
 */
void gfmLoadAsync_free(gfmLoadAsyncCtx **ppLoader) {
    /* Do nothing if there's no object */
    if (!ppLoader || !(*ppLoader)) {
        return;
    }

    /* Nothing can be done about the current asset bein loaded, but it can be
     * forced to be the last.
     *
     * Note that this is dumb, as the struct is soon freed, and it shouldn't be
     * accessed anymore... oh, well... */
    (*ppLoader)->numAssets = 0;
    free(*ppLoader);
    *ppLoader = 0;
}

/**
 * Alloc a new asynchronous loader
 *
 * @param  [out]ppLoader The asynchronous loader context
 * @return               GFraMe return value
 */
gfmRV gfmLoadAsync_getNew(gfmLoadAsyncCtx **ppLoader) {
    /** The newly alloc'ed loader */
    gfmLoadAsyncCtx *pLoader;
    /** GFraMe return value */
    gfmRV rv;

    pLoader = (gfmLoadAsyncCtx*)malloc(sizeof(gfmLoadAsyncCtx));
    ASSERT(pLoader, GFMRV_ALLOC_FAILED);
    memset(pLoader, 0x0, sizeof(gfmLoadAsyncCtx));

    *ppLoader = pLoader;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check whether the loader finished running
 *
 * @param  [ in]pLoader The asynchronous loader context
 * @return              GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmLoadAsync_didFinish(gfmLoadAsyncCtx *pLoader) {
    if (!pLoader) {
        return GFMRV_ARGUMENTS_BAD;
    }
    else if (pLoader->pThread &&
            pLoader->rv == GFMRV_ASYNC_LOADER_THREAD_IS_RUNNING) {
        return GFMRV_FALSE;
    }
    else {
        pLoader->pThread = 0;
        return GFMRV_TRUE;
    }
}

/**
 * Load assets in a separated thread
 *
 * @param  [out]pProgress Updated with how many assets have been loaded
 * @param  [ in]pLoader   The asynchronous loader context
 * @param  [ in]pCtx      The lib's main context
 * @param  [ in]pType     List of assets types to be loaded
 * @param  [ in]ppPath    List of paths to the assets
 * @param  [ in]ppHandles List of pointers where the loaded handles shall be
 *                        stored
 * @param  [ in]numAssets How many assets are there to be loaded
 * @return                GFraMe return value
 */
gfmRV gfmLoadAsync_loadAssets(int *pProgress, gfmLoadAsyncCtx *pLoader,
        gfmCtx *pCtx, gfmAssetType *pType, char **ppPath, int **ppHandles,
        int numAssets) {
    /** The retrieved logger */
    gfmLog *pLog;
    /** GFraMe return value */
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    ASSERT_LOG(pProgress, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pLoader, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pType, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppPath, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppHandles, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(numAssets > 0, GFMRV_ARGUMENTS_BAD, pLog);

    /* Make sure no other thread is running */
    ASSERT_LOG(gfmLoadAsync_didFinish(pLoader) == GFMRV_TRUE,
            GFMRV_ASYNC_LOADER_THREAD_ALREADY_RUNNING, pLog);

    /* Initialize the loader context and start the thread */
    pLoader->pLog = pLog;
    pLoader->pCtx = pCtx;
    pLoader->pType = pType;
    pLoader->ppPath = ppPath;
    pLoader->ppHandles = ppHandles;
    pLoader->pProgress = pProgress;
    pLoader->numAssets = numAssets;
    pLoader->rv = GFMRV_ASYNC_LOADER_THREAD_IS_RUNNING;

    *pProgress = 0;
    pLoader->pThread = SDL_CreateThread(_gfmLoadAsync_thread,
            "GFrame_asynchronous_loader_thread", pLoader);
    ASSERT_LOG(pLoader->pThread, GFMRV_INTERNAL_ERROR, pLog);
    SDL_DetachThread(pLoader->pThread);

    rv = GFMRV_OK;
__ret:
    return rv;
}

