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
#if !defined(EMCC) && !defined(__WIN32) && !defined(__WIN32__)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

#define STACK_SIZE 4

/* Types of possible operations */
enum enGFMFileOp {
    gfmFile_noop = 0,
    gfmFile_read,
    gfmFile_write
};

struct stGFMFile {
    /** The currently opened file pointer */
    FILE *pFp;
    /** Path to the file */
    gfmString *pPath;
    /** Last read char */
    int lastChar;
    /** Current node to be popped */
    int curStackPos;
    /** Type of the last executed operation */
    enum enGFMFileOp lastOp;
    /** The actual stack */
    fpos_t stack[STACK_SIZE];
    /** Buffer for reading stuff */
    char pBuf[4];
};

/**
 * Alloc a new gfmFile struct
 * 
 * @param  ppCtxe The alloc'ed file
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmFile_getNew(gfmFile **ppCtx) {
    gfmRV rv;

    /* Sanitizer arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);

    /* Alloc the struct */
    *ppCtx = (gfmFile*)malloc(sizeof(gfmFile));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    /* Clean it */
    memset(*ppCtx, 0x0, sizeof(gfmFile));
    /* Alloc the path string */
    rv = gfmString_getNew(&((*ppCtx)->pPath));
    ASSERT(rv == GFMRV_OK, rv);

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

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Release the path */
    gfmString_free(&((*ppCtx)->pPath));
    /* Close the file (in case it's still open) */
    gfmFile_close(*ppCtx);
    /* Free the memory */
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

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(filenameLen > 0, GFMRV_ARGUMENTS_BAD);

    /* Check that the file isn't opened */
    ASSERT(pCtx->pFp == 0, GFMRV_FILE_ALREADY_OPEN);

#ifndef EMCC
    /* Retrieve the file's directory */
    rv = gfmString_getString(&pPath, pStr);
    ASSERT_NR(rv == GFMRV_OK);

    /* Initialize the local string with that path */
    rv = gfmString_init(pCtx->pPath, pPath, strlen(pPath), 1/*doCopy*/);
    ASSERT_NR(rv == GFMRV_OK);
#else
    rv = gfmString_setLength(pCtx->pPath, 0/*len*/);
    ASSERT(rv == GFMRV_OK, rv);
#endif

    /* Append the filename to its path */
    rv = gfmString_concat(pCtx->pPath, pFilename, filenameLen);
    ASSERT_NR(rv == GFMRV_OK);

    /* Retrieve the absolute path */
    rv = gfmString_getString(&pPath, pCtx->pPath);
    ASSERT_NR(rv == GFMRV_OK);

    /* Open the file */
    pCtx->pFp = fopen(pPath, mode);
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_FOUND);

    /* Clear stuff */
    pCtx->lastChar = -1;
    pCtx->curStackPos = STACK_SIZE;

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
 * @param  mode        The mode to open the file
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_ALREADY_OPEN,
 *                     GFMRV_FILE_NOT_FOUND
 */
gfmRV gfmFile_openLocal(gfmFile *pFile, gfmCtx *pCtx, char *pFilename,
        int filenameLen, const char *mode) {
    gfmRV rv;
    gfmString *pStr;

    /* Set default values */
    pStr = 0;

    /* Sanitize arguments */
    ASSERT(pFile, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the absolute file path (i.e., copy the 'static' string) */
    rv = gfm_getLocalPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Open the file */
    rv = gfmFile_openFile(pFile, pFilename, filenameLen, pStr, mode);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    gfmString_free(&pStr);

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

    /* Sanitize arguments */
    ASSERT(pFile, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the absolute file path (i.e., copy the 'static' string) */
    rv = gfm_getBinaryPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concatStatic(pStr, "assets/");
    ASSERT_NR(rv == GFMRV_OK);

    /* Open the file */
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

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the file is actually open */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    fflush(pCtx->pFp);
    fclose(pCtx->pFp);
    pCtx->pFp = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if a file is currently open
 * 
 * @param  pCtx The 'generic' file
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmFile_isOpen(gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    if (pCtx->pFp) {
        rv = GFMRV_TRUE;
    }
    else {
        rv = GFMRV_FALSE;
    }
__ret:
    return rv;
}

/**
 * Get the path to the currently opened file
 * 
 * @param  [out]ppPath The path to the file (mustn't be dealloc'ed)
 * @param  [ in]pCtx   The 'generic' file
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_getPath(char **ppPath, gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppPath, GFMRV_ARGUMENTS_BAD);
    /* Check that the file is currently open */
    ASSERT(gfmFile_isOpen(pCtx) == GFMRV_TRUE, GFMRV_FILE_NOT_OPEN);

    /* Retrieve the file's path */
    rv = gfmString_getString(ppPath, pCtx->pPath);
    ASSERT_NR(rv == GFMRV_OK);

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
gfmRV gfmFile_getSize(int *pSize, gfmFile *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Retrieve the current position into the file
 *
 * @param  [out]pPos The current position
 * @param  [ in]pCtx The file struct
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_getPos(int *pPos, gfmFile *pCtx) {
    gfmRV rv;
    long pos;

    /* Sanitize arguments */
    ASSERT(pPos, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the file was opened */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* Retrieve its position */
    pos = ftell(pCtx->pFp);
    ASSERT(pos >= 0, GFMRV_INTERNAL_ERROR);

    *pPos = pos;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if a file reached its end
 * 
 * @param  pCtx The 'generic' file
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN, GFMRV_TRUE,
 *              GFMRV_FALSE
 */
gfmRV gfmFile_didFinish(gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    if (feof(pCtx->pFp)) {
        rv = GFMRV_TRUE;
    }
    else {
        rv = GFMRV_FALSE;
    }
__ret:
    return rv;
}

/**
 * Rewind a file back to its start
 * 
 * @param  pCtx  The file struct
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_rewind(gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    rewind(pCtx->pFp);
    pCtx->lastOp = gfmFile_noop;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Move a few bytes forward/backward from the current position
 * 
 * @param  pCtx     The file struct
 * @param  numBytes How many bytes should be seeked
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *                  GFMRV_INTERNAL_ERROR
 */
gfmRV gfmFile_seek(gfmFile *pCtx, int numBytes) {
    gfmRV rv;
    int irv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    irv = fseek(pCtx->pFp, numBytes, SEEK_CUR);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->lastOp = gfmFile_noop;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Flush the file
 * 
 * @param  pCtx The file struct
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_INTERNAL_ERROR
 */
gfmRV gfmFile_flush(gfmFile *pCtx) {
    gfmRV rv;
    int irv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);
    /* TODO Check that it was open for writing */

    irv = fflush(pCtx->pFp);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->lastOp = gfmFile_noop;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many 'nodes' left there are on the stack
 * 
 * @param  pNum The number of 'nodes' left
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_getPosStackLeft(int *pNum, gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguents */
    ASSERT(pNum, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    *pNum = pCtx->curStackPos;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Push the current position into a stack (really useful for parsing stuff)
 * 
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_FILE_MAX_STACK_POS, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmFile_pushPos(gfmFile *pCtx) {
    gfmRV rv;
    int irv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);
    /* Check that there are spaces left at the stack */
    ASSERT(pCtx->curStackPos > 0, GFMRV_FILE_MAX_STACK_POS);

    /* Push the position into the stack */
    pCtx->curStackPos--;
    irv = fgetpos(pCtx->pFp, &(pCtx->stack[pCtx->curStackPos]));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->lastOp = gfmFile_noop;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pop the previous position from the stack (really useful for parsing stuff)
 * 
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_FILE_STACK_EMPTY, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmFile_popPos(gfmFile *pCtx) {
    gfmRV rv;
    int irv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);
    /* Check that the stack isn't empty */
    ASSERT(pCtx->curStackPos < STACK_SIZE, GFMRV_FILE_STACK_EMPTY);

    /* Move the stream to the previous position */
    irv = fsetpos(pCtx->pFp, &(pCtx->stack[pCtx->curStackPos]));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    /* Pop it from the stack */
    pCtx->curStackPos++;
    pCtx->lastOp = gfmFile_noop;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Move the stack a single position back, but doesn't pop it (i.e., doesn't
 * "rewind" to that position)
 * 
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_clearLastPosStack(gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    if (pCtx->curStackPos < STACK_SIZE) {
        pCtx->curStackPos++;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear the 'position stack'
 * 
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN
 */
gfmRV gfmFile_clearPosStack(gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    pCtx->curStackPos = STACK_SIZE;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Read a character from the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_FILE_EOF_REACHED
 */
gfmRV gfmFile_readChar(char *pVal, gfmFile *pCtx) {
    gfmRV rv;

    /* Sanitize arguents */
    ASSERT(pVal, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* ANSI C requirement... */
    if (pCtx->lastOp == gfmFile_write) {
        fseek(pCtx->pFp, 0L, SEEK_CUR);
    }
    pCtx->lastOp = gfmFile_read;

    /* Read a char */
    pCtx->lastChar = fgetc(pCtx->pFp);
    if (pCtx->lastChar == EOF) {
        rv = GFMRV_FILE_EOF_REACHED;
        goto __ret;
    }
    /*ASSERT(pCtx->lastChar != EOF, GFMRV_FILE_EOF_REACHED); */

    *pVal = (char)pCtx->lastChar;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write a character to the file
 * 
 * @param  pVal The character
 * @param  pCtx The file
 */
gfmRV gfmFile_writeChar(gfmFile *pCtx, unsigned char val) {
    gfmRV rv;
    int irv;

    /* Sanitize arguents */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* ANSI C requirement... */
    if (pCtx->lastOp == gfmFile_read) {
        fseek(pCtx->pFp, 0L, SEEK_CUR);
    }
    pCtx->lastOp = gfmFile_write;

    /* Read a char */
    irv = fputc(val, pCtx->pFp);
    ASSERT(irv == (int)((unsigned char)val), GFMRV_FILE_WRITE_ERROR);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Roll back a character (similar to stdio's ungetc); The last read character
 * (if needed) must be retrieved from the file context
 * NOTE: This function doesn't need to allow more than one unread (but it can!)
 * 
 * @param  pCtx The file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_FILE_CANT_UNREAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmFile_unreadChar(gfmFile *pCtx) {
    gfmRV rv;
    int irv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);
    /* Check that there's a character to be unread */
    ASSERT(pCtx->lastChar >= 0, GFMRV_FILE_CANT_UNREAD);

    /* Unread the character */
    irv = ungetc(pCtx->lastChar, pCtx->pFp);
    ASSERT(irv == pCtx->lastChar, GFMRV_INTERNAL_ERROR);

    pCtx->lastChar = -1;
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_READ_ERROR
 */
gfmRV gfmFile_readWord(int *pVal, gfmFile *pCtx) {
    gfmRV rv;
    int irv, count;

    /* Sanitize arguments */
    ASSERT(pVal, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* ANSI C requirement... */
    if (pCtx->lastOp == gfmFile_write) {
        fseek(pCtx->pFp, 0L, SEEK_CUR);
    }
    pCtx->lastOp = gfmFile_read;

    /* Try to read a integer (i.e., 4 bytes) */
    count = 4;
    irv = fread(pCtx->pBuf, sizeof(char), count, pCtx->pFp);
    ASSERT(irv == count, GFMRV_READ_ERROR);
    /* Convert the word (the arch is expected to be little-endian) */
    *pVal = (int)( ( ((pCtx->pBuf)[0]      ) & 0x000000ff)
            | ( ((pCtx->pBuf)[1] << 8 ) & 0x0000ff00)
            | ( ((pCtx->pBuf)[2] << 16) & 0x00ff0000)
            | ( ((pCtx->pBuf)[3] << 24) & 0xff000000) );

    pCtx->lastChar = -1;
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *                  GFMRV_READ_ERROR, GFMRV_FILE_EOF_REACHED
 */
gfmRV gfmFile_readBytes(char *pVal, int *pLen, gfmFile *pCtx, int numBytes) {
    gfmRV rv;
    int count;

    /* Sanitize arguments */
    ASSERT(pVal, GFMRV_ARGUMENTS_BAD);
    ASSERT(pLen, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* ANSI C requirement... */
    if (pCtx->lastOp == gfmFile_write) {
        fseek(pCtx->pFp, 0L, SEEK_CUR);
    }
    pCtx->lastOp = gfmFile_read;

    count = fread(pVal, sizeof(char), numBytes, pCtx->pFp);
    ASSERT(count >= 0, GFMRV_READ_ERROR);

    *pLen = count;
    pCtx->lastChar = -1;
    if (count == 0) {
        rv = GFMRV_FILE_EOF_REACHED;
    }
    else {
        rv = GFMRV_OK;
    }
__ret:
    return rv;
}

/**
 * Read a stream of bytes from the file; If the EOF is reached before reading
 * the desired number of bytes, this function must return successfuly
 * 
 * @param  pCtx The file
 * @param  pVal A array of bytes of numBytes length
 * @param  len  How many bytes were actually read from the file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FILE_NOT_OPEN,
 *              GFMRV_FILE_WRITE_ERROR
 */
gfmRV gfmFile_writeBytes(gfmFile *pCtx, char *pVal, int len) {
    gfmRV rv;
    int count;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pVal, GFMRV_ARGUMENTS_BAD);
    ASSERT(len, GFMRV_ARGUMENTS_BAD);
    /* Check that there's an open file */
    ASSERT(pCtx->pFp, GFMRV_FILE_NOT_OPEN);

    /* ANSI C requirement... */
    if (pCtx->lastOp == gfmFile_read) {
        fseek(pCtx->pFp, 0L, SEEK_CUR);
    }
    pCtx->lastOp = gfmFile_write;

    count = fwrite(pVal, sizeof(char), len, pCtx->pFp);
    ASSERT(count == len, GFMRV_FILE_WRITE_ERROR);

    rv = GFMRV_OK;
__ret:
    return rv;
}

