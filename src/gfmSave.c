/**
 * Abstraction for save files.
 *
 * It's written over the gfmFile module. The only thing it actually does is
 * store key, type, value tuples.
 *
 * @file src/gfmSave.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmSave.h>
#include <GFraMe/core/gfmFile_bkend.h>

#include <stdlib.h>
#include <string.h>

/** gfmSave struct */
struct stGFMSave {
    /* Save file version (useful if this is ever modified) */
    int version;
    /** The actual save file */
    gfmFile *pFile;
    /** Logger, so we can debug stuff */
    gfmLog *pLog;
};

/**
 * Alloc a new save object
 *
 * @param  [out]ppCtx The alloc'ed object
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSave_getNew(gfmSave **ppCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Alloc the struct */
    *ppCtx = (gfmSave*)malloc(sizeof(gfmSave));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);

    /* Clean it */
    memset(*ppCtx, 0x0, sizeof(gfmSave));

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a save object, flushing and releasing all of its resources
 *
 * @param  [ in]ppCtx The object to be free'd
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_free(gfmSave **ppCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);

    /* TODO Release all internal structs */
    gfmFile_free(&((*ppCtx)->pFile));

    /* Release the save structure */
    free(*ppCtx);
    *ppCtx = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Associate a save object with a file on the system
 *
 * @param  [ in]pSave     The object to be free'd
 * @param  [ in]pCtx      The game context
 * @param  [ in]pFilename Name of the save file
 * @param  [ in]len       Length of the filename
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_bind(gfmSave *pSave, gfmCtx *pCtx, char *pFilename, int len) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pSave, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the logger */
    rv = gfm_getLogger(&(pSave->pLog), pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /* Open the save file */
    rv = gfmFile_getNew(&(pSave->pFile));
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);
    /* Try to open the file for read/write (file must exist) */
    rv = gfmFile_openLocal(pSave->pFile, pCtx, pFilename, len, "r+b");
    ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_FILE_NOT_FOUND, rv, pSave->pLog);
    if (rv == GFMRV_FILE_NOT_FOUND) {
        /* If it didn't already exist, create it */
        rv = gfmFile_openLocal(pSave->pFile, pCtx, pFilename, len, "w+b");
        ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

        /* Add save file version */
        rv = gfmSave_writeStatic(pSave, "gfmSave", 0x00010000);
        ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

        /* Close and re-open the file in "r+b" mode; This is necessary because
         * of the way that erasing a file is handled by gfmFile */
        rv = gfmFile_close(pSave->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

        rv = gfmFile_openLocal(pSave->pFile, pCtx, pFilename, len, "r+b");
    }
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

    /* Check the save file version */
    rv = gfmSave_readStatic(&(pSave->version), pSave, "gfmSave");
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Flush the save file and close it
 *
 * @param  [ in]pCtx The object to be free'd
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_close(gfmSave *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Close the file */
    if (pCtx->pFile) {
        rv = gfmFile_close(pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Erase all contents of the save file
 *
 * NOTE: The file is kept open for read/write operations
 *
 * @param  [ in]pCtx The object to be free'd
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSave_erase(gfmSave *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Store a tuple on the save file
 *
 * @param  [ in]pCtx  The save file
 * @param  [ in]pId   Tuple's key
 * @param  [ in]len   Tuple's value
 * @param  [ in]value Value to be stored
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSave_write(gfmSave *pCtx, char *pId, int len, int value) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pId, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the save file is bound */
    ASSERT(pCtx->pFile, GFMRV_SAVE_NOT_BOUND);

    /* TODO Check if the id already exists */
    /* TODO If not, write the id */
    /* TODO Else, seek its position */
    /* TODO Write the value */

    return GFMRV_FUNCTION_NOT_IMPLEMENTED;

    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int numbytes);

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

