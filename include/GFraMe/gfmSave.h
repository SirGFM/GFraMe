/**
 * Abstraction for save files.
 *
 * It's written over the gfmFile module. The only thing it actually does is
 * store key, type, value tuples.
 *
 * @file include/GFraMe/gfmSave.h
 */
#ifndef __GFMSAVE_STRUCT__
#define __GFMSAVE_STRUCT__

/** Export the 'gfmSave' struct */
typedef struct stGFMSave gfmSave;

#endif /* __GFMSAVE_STRUCT__ */

#ifndef __GFMSAVE_H__
#define __GFMSAVE_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Alloc a new save object
 *
 * @param  [out]ppCtx The alloc'ed object
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSave_getNew(gfmSave **ppCtx);

/**
 * Free a save object, flushing and releasing all of its resources
 *
 * @param  [ in]ppCtx The object to be free'd
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_free(gfmSave **ppCtx);

#define gfmSave_bindStatic(pSave, pCtx, pFilename) \
    gfmSave_bind(pSave, pCtx, pFilename, sizeof(pFilename) - 1)

/**
 * Associate a save object with a file on the system
 *
 * @param  [ in]pSave     The object to be free'd
 * @param  [ in]pCtx      The game context
 * @param  [ in]pFilename Name of the save file
 * @param  [ in]len       Length of the filename
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_bind(gfmSave *pSave, gfmCtx *pCtx, char *pFilename, int len);

/**
 * Flush the save file and close it
 *
 * @param  [ in]pCtx The object to be free'd
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_close(gfmSave *pCtx);

/**
 * Erase all contents of the save file
 *
 * NOTE: The file is kept open for read/write operations
 *
 * @param  [ in]pCtx The object to be free'd
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_erase(gfmSave *pCtx);

#define gfmSave_writeStatic(pCtx, pId, value) \
    gfmSave_write(pCtx, pId, sizeof(pId) - 1, value)

/**
 * Store a tuple on the save file
 *
 * @param  [ in]pCtx  The save file
 * @param  [ in]pId   Tuple's key
 * @param  [ in]len   Tuple's value
 * @param  [ in]value Value to be stored
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_write(gfmSave *pCtx, char *pId, int len, int value);

#define gfmSave_readStatic(pValue, pCtx, pId) \
    gfmSave_read(pValue, pCtx, pId, sizeof(pId) - 1)

/**
 * Retrieve a tuple from the save file
 *
 * @param  [out]pValue Value to be stored
 * @param  [ in]pCtx   The save file
 * @param  [ in]pId    Tuple's key
 * @param  [ in]len    Tuple's value
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_read(int *pValue, gfmSave *pCtx, char *pId, int len);

#define gfmSave_writeDataStatic(pCtx, pId, value, numBytes) \
    gfmSave_writeData(pCtx, pId, sizeof(pId) - 1, value, numBytes)

/**
 * Store an array of bytes on the save file
 *
 * @param  [ in]pCtx     The save file
 * @param  [ in]pId      Tuple's key
 * @param  [ in]len      Tuple's value
 * @param  [ in]pData    Data to be stored
 * @param  [ in]numBytes Number of bytes in pData
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_writeData(gfmSave *pCtx, char *pId, int len, char *pData,
        int numBytes);

#define gfmSave_readDataStatic(pValue, pNumBytes, pCtx, pId) \
    gfmSave_readData(pValue, pNumBytes, pCtx, pId, sizeof(pId) - 1)

/**
 * Retrieve an array of bytes from the save file
 *
 * NOTE 1: If pData is NULL, pNumBytes will return the length of the requested
 * tuple
 *
 * NOTE 2: If pData isn't NULL, it must be at least 'pNumBytes' long (therefore,
 * it should usually be called twice)
 *
 * @param  [out]pData     Data to be stored
 * @param  [out]pNumBytes Length of the stored data
 * @param  [ in]pCtx      The save file
 * @param  [ in]pId       Tuple's key
 * @param  [ in]len       Tuple's value
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_readData(char *pData, int *pNumBytes, gfmSave *pCtx, char *pId,
        int len);

#endif /* __GFMSAVE_H__ */

