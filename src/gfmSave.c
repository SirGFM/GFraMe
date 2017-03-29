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

/* Tuple's info (except for it's data) */
struct stGFMSaveTuple {
    /* The tuple's ID */
    char pId[256];
    /* ID's length */
    unsigned char idLen;
    /* Number of bytes on the data */
    unsigned char dataLen;
};
typedef struct stGFMSaveTuple gfmSaveTuple;

/** gfmSave struct */
struct stGFMSave {
    /* Save file version (useful if this is ever modified) */
    int version;
    /** The actual save file */
    gfmFile *pFile;
    /** Logger, so we can debug stuff */
    gfmLog *pLog;
    /* Tuple at which the file is currently pointing */
    gfmSaveTuple curTuple;
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

    rv = gfmLog_log(pSave->pLog, gfmLog_debug, "Binding \"%*s\" to a save "
            "file...", len, pFilename);
    ASSERT(rv == GFMRV_OK, rv);

    /* Open the save file */
    rv = gfmFile_getNew(&(pSave->pFile));
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);
    /* Try to open the file for read/write (file must exist) */
    rv = gfmFile_openLocal(pSave->pFile, pCtx, pFilename, len, "r+b");
    ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_FILE_NOT_FOUND, rv, pSave->pLog);
    if (rv == GFMRV_FILE_NOT_FOUND) {
        rv = gfmLog_log(pSave->pLog, gfmLog_debug, "Creating a new file...");
        ASSERT(rv == GFMRV_OK, rv);

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

        /* Since the file was closed and the file is back at the start, the
         * cache must be flushed */
        memset(&(pSave->curTuple), 0x0, sizeof(gfmSaveTuple));

        rv = gfmFile_openLocal(pSave->pFile, pCtx, pFilename, len, "r+b");
    }
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

    /* Check the save file version */
    rv = gfmSave_readStatic(&(pSave->version), pSave, "gfmSave");
    ASSERT_LOG(rv == GFMRV_OK, rv, pSave->pLog);

    rv = gfmLog_log(pSave->pLog, gfmLog_debug, "File opened! Version 0x%x",
            pSave->version);
    ASSERT(rv == GFMRV_OK, rv);

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
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

        rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Save file closed");
        ASSERT(rv == GFMRV_OK, rv);

        /* Make sure there's no cached tuple */
        memset(&(pCtx->curTuple), 0x0, sizeof(gfmSaveTuple));
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
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Erasing file...");
    ASSERT(rv == GFMRV_OK, rv);

    /* Erase the file */
    rv = gfmFile_erase(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Make sure there's no cached tuple */
    memset(&(pCtx->curTuple), 0x0, sizeof(gfmSaveTuple));

    /* Add save file version */
    rv = gfmSave_writeStatic(pCtx, "gfmSave", 0x00010000);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Compare an ID to a tuple
 *
 * @param  [ in]pCtx The tuple
 * @param  [ in]pId  The key
 * @param  [ in]len  The key's length
 * @return           GFMRV_TRUE, GFMRV_FALSE
 */
static gfmRV gfmSave_tuplecmp(gfmSaveTuple *pCtx, char *pId, int len) {
    int i;

    /* No need to sanitize as inputs have been previously sanitized */

    /* If the sizes aren't equal, the string definitely differ */
    if (pCtx->idLen != len) {
        return GFMRV_FALSE;
    }

    i = 0;
    while (i < len) {
        /* Check that every character match */
        if (pId[i] != pCtx->pId[i]) {
            return GFMRV_FALSE;
        }
        i++;
    }

    return GFMRV_TRUE;
}

/**
 * Search for the desired ID
 *
 * @param  [ in]pCtx The save file
 * @param  [ in]pId  Tuple's key
 * @return           GFMRV_OK, GFMRV_SAVE_ID_NOT_FOUND
 */
gfmRV gfmSave_findId(gfmSave *pCtx, char *pId, int len) {
    gfmSaveTuple *pTuple;
    gfmRV rv;
    int didLoop, loopPos, pos, size;

    /* Sanitize arguments */
    pTuple = 0;
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pId, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len < 128, GFMRV_SAVE_ID_TOO_LONG, pCtx->pLog);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    /* Retrieve the current tuple */
    pTuple = &(pCtx->curTuple);

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Searching tuple \"%*s\"...", len,
            pId);
    ASSERT(rv == GFMRV_OK, rv);

    /* Check if the files is already at the desired tuple */
    rv = gfmSave_tuplecmp(pTuple, pId, len);
    if (rv == GFMRV_TRUE) {
        rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Already pointing to it!");
        ASSERT(rv == GFMRV_OK, rv);
        return GFMRV_OK;
    }
    else if (pTuple->dataLen > 0) {
        /* If the current tuple isn't the desired one, move to the start (i.e.,
         * its label's length) of the next one */
        rv = gfmFile_seek(pCtx->pFile, pTuple->dataLen);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }

    /* Get the file's length and current position */
    rv = gfmFile_getSize(&size, pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmFile_getPos(&loopPos, pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Current file size: %i", size);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Starting from positiong %i...",
            loopPos);
    ASSERT(rv == GFMRV_OK, rv);

    /* Search for the tuple */
    didLoop = 0;
    pos = loopPos;
    while (1) {
        int len;

        /* Check if we just looped over the file */
        if (!didLoop && pos >= size) {
            rv = gfmFile_rewind(pCtx->pFile);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
            pos = 0;
            didLoop = 1;

            rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Just looped!");
            ASSERT(rv == GFMRV_OK, rv);
        }
        /* No 'else' here so it can break on the first pass */
        if (didLoop && pos >= loopPos) {
            rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Failed to find ID");
            ASSERT(rv == GFMRV_OK, rv);
            rv = GFMRV_SAVE_ID_NOT_FOUND;
            break;
        }

        /* Retrieve the current ID */
        rv = gfmFile_readChar((char*)&(pTuple->idLen), pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        pos++;
        rv = gfmFile_readBytes(pTuple->pId, &len, pCtx->pFile, pTuple->idLen);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        pos += len;
        pTuple->idLen = (unsigned char)len;

        /* Retrieve the data length */
        rv = gfmFile_readChar((char*)&(pTuple->dataLen), pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        pos++;

        rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Just found tuple \"%*s\" "
                "(dataLen: %i)", pTuple->idLen, pTuple->pId, pTuple->dataLen);
        ASSERT(rv == GFMRV_OK, rv);

        /* Check if it's the desired id */
        rv = gfmSave_tuplecmp(pTuple, pId, len);
        if (rv == GFMRV_TRUE) {
            rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Tuple found!");
            ASSERT(rv == GFMRV_OK, rv);
            rv = GFMRV_OK;
            break;
        }

        /* If it wasn't found, move to the next one */
        pos += pTuple->dataLen;
        rv = gfmFile_seek(pCtx->pFile, pTuple->dataLen);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }

__ret:
    if (rv != GFMRV_OK && pTuple) {
        /* Clear the current tuple, as it's invalid */
        memset(pTuple, 0x0, sizeof(gfmSaveTuple));
    }

    return rv;
}

/**
 * Write a ID at the end of the file
 *
 * @param  [ in]pCtx The save file
 * @param  [ in]pId  Tuple's key
 * @param  [ in]len  Tuple's value
 * @return           GFMRV_OK, ...
 */
static gfmRV gfmSave_writeID(gfmSave *pCtx, char *pId, int len) {
    gfmRV rv;
    int size;

    /* No need to sanitize as inputs have been previously sanitized */

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Adding ID \"%*s\"...", len, pId);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmFile_getSize(&size, pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Move to the end of the file */
    rv = gfmFile_rewind(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmFile_seek(pCtx->pFile, size);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Store the id length */
    rv = gfmFile_writeChar(pCtx->pFile, len);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    /* Store the actual id */
    rv = gfmFile_writeBytes(pCtx->pFile, pId, len);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Store the current tuple */
    strncpy(pCtx->curTuple.pId, pId, 256);
    pCtx->curTuple.pId[255] = '\0';
    pCtx->curTuple.idLen = len;

    rv = GFMRV_OK;
__ret:
    return rv;
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
    ASSERT_LOG(pId, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len < 128, GFMRV_SAVE_ID_TOO_LONG, pCtx->pLog);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Writing value (\"%*s\", %i) to "
            "file", len, pId, value);
    ASSERT(rv == GFMRV_OK, rv);

    /* Check if the id already exists */
    rv = gfmSave_findId(pCtx, pId, len);
    if (rv == GFMRV_SAVE_ID_NOT_FOUND) {
        /* If not, write the id */
        rv = gfmSave_writeID(pCtx, pId, len);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

        /* Set the tuple len (always 4 bytes, for this operation) */
        pCtx->curTuple.dataLen = 4;

        /* Write the value's length (always 4 bytes) */
        rv = gfmFile_writeChar(pCtx->pFile, pCtx->curTuple.dataLen);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    /* Write the value */
    rv = gfmFile_writeWord(pCtx->pFile, value);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Move the file back to the start of this tuple's data segment */
    rv = gfmFile_seek(pCtx->pFile, -pCtx->curTuple.dataLen);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

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
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SAVE_ID_NOT_FOUND
 */
gfmRV gfmSave_read(int *pValue, gfmSave *pCtx, char *pId, int len) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pValue, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pId, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len < 128, GFMRV_SAVE_ID_TOO_LONG, pCtx->pLog);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Retrieving \"%*s\"'s value...",
            len, pId);
    ASSERT(rv == GFMRV_OK, rv);

    /* Check if the id exists */
    rv = gfmSave_findId(pCtx, pId, len);
    if (rv == GFMRV_OK) {
        rv = gfmFile_readWord(pValue, pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

        rv = gfmLog_log(pCtx->pLog, gfmLog_debug, "Found value %i", *pValue);
        ASSERT(rv == GFMRV_OK, rv);

        /* Move the file back to the start of this tuple's data segment */
        rv = gfmFile_seek(pCtx->pFile, -pCtx->curTuple.dataLen);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
__ret:
    return rv;
}

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
        int numBytes) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pId, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len < 128, GFMRV_SAVE_ID_TOO_LONG, pCtx->pLog);
    ASSERT_LOG(pData, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(numBytes > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(numBytes < 128, GFMRV_SAVE_DATA_TOO_LONG, pCtx->pLog);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    /* Check if the id already exists */
    rv = gfmSave_findId(pCtx, pId, len);
    if (rv == GFMRV_SAVE_ID_NOT_FOUND) {
        /* If not, write the id */
        rv = gfmSave_writeID(pCtx, pId, len);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

        /* Write the value's length */
        rv = gfmFile_writeChar(pCtx->pFile, numBytes);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

        /* Set the tuple len */
        pCtx->curTuple.dataLen = numBytes;
    }
    else {
        /* Check that the size wasn't modified */
        ASSERT_LOG(numBytes == pCtx->curTuple.dataLen,
                GFMRV_SAVE_INVALID_DATA_LEN, pCtx->pLog);
    }
    /* Write the value */
    rv = gfmFile_writeBytes(pCtx->pFile, pData, numBytes);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    /* Move the file back to the start of this tuple's data segment */
    rv = gfmFile_seek(pCtx->pFile, -pCtx->curTuple.dataLen);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);

    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int len) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pData, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pNumBytes, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pId, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len < 128, GFMRV_SAVE_ID_TOO_LONG, pCtx->pLog);
    /* Check that the save file is bound */
    ASSERT_LOG(pCtx->pFile, GFMRV_SAVE_NOT_BOUND, pCtx->pLog);

    /* Check if the id exists */
    rv = gfmSave_findId(pCtx, pId, len);
    if (rv == GFMRV_OK) {
        /* Check if the data should actually be read */
        if (pData) {
            int len;

            rv = gfmFile_readBytes(pData, &len, pCtx->pFile,
                    pCtx->curTuple.dataLen);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
            ASSERT_LOG(len == pCtx->curTuple.dataLen, GFMRV_INTERNAL_ERROR,
                    pCtx->pLog);

            /* Move the file back to the start of this tuple's data segment */
            rv = gfmFile_seek(pCtx->pFile, -pCtx->curTuple.dataLen);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        }

        /* Retrieve the data length */
        *pNumBytes = pCtx->curTuple.dataLen;
    }
__ret:
    return rv;
}

