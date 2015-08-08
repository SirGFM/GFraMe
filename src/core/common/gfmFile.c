/**
 * @file src/core/commmon/gfmFile.c
 * 
 * Generic file interface; It should be used to abstract opening assests (which
 * might be compressed, on mobile) and opening a file (e.g., the Log) on the
 * default path (e.g., ~/.local/share/<company>/<game> or
 * %APPDATA%\<company>\<game>)
 * NOTE: This implementation shouldn't be used for mobile!
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmFile_bkend.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Includes for checking a file size on non-Windows */
#if !defined(__WIN32) && !defined(__WIN32__)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

struct stGFMFile {
    /** The currently opened file pointer */
    FILE *pFp;
};

/**
 * Alloc a new gfmFile struct
 * 
 * @param  ppCtxe The alloc'ed file
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmFile_getNew(gfmFile **ppCtx) {
    gfmRV rv;
    
    // Sanitizer arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the struct
    *ppCtx = (gfmFile*)malloc(sizeof(gfmFile));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmFile));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Close and free a gfmFile
 * 
 * @param  ppCtx The file
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFile_free(gfmFile **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Close the file (in case it's still open)
    gfmFile_close(*ppCtx);
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Open a file
 *
 * @param  pCtx        The file struct
 * @param  pFilename   The filename
 * @param  filenameLen Length of the filename
 * @param  pStr        gfmString with the file's directory
 * @param  mode        Mode to open the file
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_ALREADY_OPEN,
 *                     GFMRV_FILE_NOT_FOUND
 */
static gfmRV gfmFile_openFile(gfmFile *pCtx, char *pFilename, int filenameLen,
        gfmString *pStr, const char *mode) {
    char *pPath;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(filenameLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check that the file isn't opened
    ASSERT(pCtx->pFp == 0, GFMRV_FILE_ALREADY_OPEN);
    
    // Append the filename to its path
    rv = gfmString_concat(pStr, pFilename, filenameLen);
    ASSERT_NR(rv == GFMRV_OK);
    // Retrieve it
    rv = gfmString_getString(&pPath, pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file
    pCtx->pFp = fopen(pPath, mode);
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_FOUND);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Open a file on the game's local path. It changes with the OS:
 *   - android: ???
 *   - linux: ~/.local/share/<company>/<game>/
 *   - windows: %APPDATA%\<company>\<game>\
 * 
 * @param  pFile       The file struct
 * @param  pCtx        The game's context
 * @param  pFilename   The filename
 * @param  filenameLen Length of the filename
 * @param  isText      Whether the asset file is a text file
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_ALREADY_OPEN,
 *                     GFMRV_FILE_NOT_FOUND
 */
gfmRV gfmFile_openLocal(gfmFile *pFile, gfmCtx *pCtx, char *pFilename,
        int filenameLen, int isText) {
    gfmRV rv;
    gfmString *pStr;
    
    // Sanitize arguments
    ASSERT(pFile, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the absolute file path
    rv = gfm_getLocalPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file
    if (isText) {
        rv = gfmFile_openFile(pFile, pFilename, filenameLen, pStr, "rt+");
    }
    else {
        rv = gfmFile_openFile(pFile, pFilename, filenameLen, pStr, "rb+");
    }
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Open an asset file; The file is expected to be found on an 'assets'
 * directory, which must be found on the same directory as the game's binary;
 * Note that the file will be opened only for reading!
 * 
 * @param  pFile       The file struct
 * @param  pCtx        The game's context
 * @param  pFilename   The filename
 * @param  filenameLen Length of the filename
 * @param  isText      Whether the asset file is a text file
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_ALREADY_OPEN,
 *                     GFMRV_FILE_NOT_FOUND
 */
gfmRV gfmFile_openAsset(gfmFile *pFile, gfmCtx *pCtx, char *pFilename,
        int filenameLen, int isText) {
    gfmRV rv;
    gfmString *pStr;
    
    // Sanitize arguments
    ASSERT(pFile, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the absolute file path
    rv = gfm_getBinaryPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concatStatic(pStr, "assets/");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file
    if (isText) {
        rv = gfmFile_openFile(pFile, pFilename, filenameLen, pStr, "rt");
    }
    else {
        rv = gfmFile_openFile(pFile, pFilename, filenameLen, pStr, "rb");
    }
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Close a file
 * 
 * @param  pCtx The file struct
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_close(gfmFile *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the file is actually open
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);
    
    fclose(pCtx->pFp);
    pCtx->pFp = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the file's size
 * 
 * @param  pSize The file size (in bytes)
 * @param  pCtx  The file struct
 */
gfmRV gfmFile_getSize(int *pSize, gfmFile *pCtx);

/**
 * Read a character from the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 */
gfmRV gfmFile_readChar(char *pVal, gfmFile *pCtx);

/**
 * Write a character to the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 */
gfmRV gfmFile_writeChar(gfmFile *pCtx, char val);

/**
 * Roll back a character (similar to stdio's ungetc); The last read character
 * (if needed) must be retrieved from the file context
 * 
 * @param  pCtx The file
 */
gfmRV gfmFile_unreadChar(gfmFile *pCtx);

/**
 * Read 2 bytes (i.e., half a 32 bits word) into an integer; The value is
 * expected to be in little-endian format
 * 
 * @param  pVal The half word
 * @param  pCtx The file
 */
gfmRV gfmFile_readHalfWord(int *pVal, gfmFile *pCtx);

/**
 * Read 2 bytes (i.e., half a 32 bits word) into an integer; The value will
 * be written in little-endian format
 * 
 * @param  pCtx The file
 * @param  val  The half word
 */
gfmRV gfmFile_writeHalfWord(gfmFile *pCtx, int val);

/**
 * Read 4 bytes (i.e., a 32 bits integer) into an integer; The value is
 * expected to be in little-endian format
 * 
 * @param  pVal The word
 * @param  pCtx The file
 */
gfmRV gfmFile_readWord(int *pVal, gfmFile *pCtx);

/**
 * Read 4 bytes (i.e., a 32 bits integer) into an integer; The value will be
 * written in little-endian format
 * 
 * @param  pCtx The file
 * @param  val  The word
 */
gfmRV gfmFile_writeWord(gfmFile *pCtx, int val);

/**
 * Read a stream of bytes from the file; If the EOF is reached before reading
 * the desired number of bytes, this function must return successfuly
 * 
 * @param  pVal     A array of bytes of numBytes length
 * @param  pLen     How many bytes were actually read from the file
 * @param  pCtx     The file
 * @param  numBytes How many bytes should be read
 */
gfmRV gfmFile_readBytes(char *pVal, int *pLen, gfmFile *pCtx, int numBytes);

/**
 * Read a stream of bytes from the file; If the EOF is reached before reading
 * the desired number of bytes, this function must return successfuly
 * 
 * @param  pCtx The file
 * @param  pVal A array of bytes of numBytes length
 * @param  len  How many bytes were actually read from the file
 */
gfmRV gfmFile_writeBytes(gfmFile *pCtx, char *pVal, int len);

