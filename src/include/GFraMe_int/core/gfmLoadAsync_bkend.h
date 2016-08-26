/**
 * @file src/include/GFraMe_int/core/loadAsync/gfmLoadAsync_bkend.h
 *
 * Load assets asynchronously. This implementation uses SDL2 for threading.
 */
#ifndef __GFMLOADASYNC_BKEND_STRUCT__
#define __GFMLOADASYNC_BKEND_STRUCT__

typedef struct stGFMLoadAsyncCtx gfmLoadAsyncCtx;

#endif /* __GFMLOADASYNC_BKEND_STRUCT__ */

#ifndef __GFMLOADASYNC_BKEND_H__
#define __GFMLOADASYNC_BKEND_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gframe.h>

/**
 * Finishes the loader thread and release all memory
 *
 * @param  [ in]ppLoader  The asynchronous loader context
 */
void gfmLoadAsync_free(gfmLoadAsyncCtx **ppLoader);

/**
 * Alloc a new asynchronous loader
 *
 * @param  [out]ppLoader The asynchronous loader context
 * @return               GFraMe return value
 */
gfmRV gfmLoadAsync_getNew(gfmLoadAsyncCtx **ppLoader);

/**
 * Check whether the loader finished running
 *
 * @param  [ in]pLoader The asynchronous loader context
 * @return              GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmLoadAsync_didFinish(gfmLoadAsyncCtx *pLoader);

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
        int numAssets);

#endif /* __GFMLOADASYNC_BKEND_H__ */

