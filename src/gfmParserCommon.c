/**
 * @file src/gfmParserCommon.c
 * 
 * Common functions used when parsing files
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe_int/gfmParserCommon.h>

#include <stdlib.h>

/**
 * Advance through all blank characters
 * 
 * @param  pFp The file to be read
 * @return     GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmParser_ignoreBlank(gfmFile *pFp) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    
    // Loop through all characters
    while (1) {
        char c;
        
        // Read the current character
        rv = gfmFile_readChar(&c, pFp);
        ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_FILE_EOF_REACHED);
        if (rv == GFMRV_FILE_EOF_REACHED) {
            break;
        }
        
        // Stop if it's not a blank char
        if (!gfmParser_isBlank(c)) {
            // 'Unread' the character
            rv = gfmFile_unreadChar(pFp);
            ASSERT_NR(rv == GFMRV_OK);
            // and stop
            break;
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Try to read a string from a file; On error, return the file to the previous
 * position
 * 
 * @param  ppStr   The parsed string (it may be pre-allocated, but its size must
 *                 be accurate(i.e., the buffer's size), in that case)
 * @param  pStrLen The string's length
 * @param  pFp     The current file
 * @return         GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_INTERNAL_ERROR, GFMRV_READ_ERROR,
 *                 GFMRV_TILEMAP_PARSING_ERROR, GFMRV_ALLOC_FAILED
 */
gfmRV gfmParser_getString(char **ppStr, int *pStrLen, gfmFile *pFp) {
    gfmRV rv;
    int isPosSet, len;
    
    // Set default values
    isPosSet = 0;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    ASSERT(pStrLen, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    // Either there's no string or the length is set
    ASSERT(!(*ppStr) || *pStrLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Get the current position, to "backtrack" on error
    rv = gfmFile_pushPos(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    isPosSet = 1;
    
    // Read all characters until a blank one
    len = 0;
    while (1) {
        char c;
        
        // Read the next character
        rv = gfmFile_readChar(&c, pFp);
        ASSERT_NR(rv == GFMRV_OK);
        // Stop if it's a blank character
        if (gfmParser_isBlank(c)) {
            break;
        }
        // Expand the buffer as necessary
        if (len >= *pStrLen) {
            // Realloc the buffer with enough space for more chars and the null
            *ppStr = (char*)realloc(*ppStr, (*pStrLen) * 2 + 1);
            ASSERT(*ppStr, GFMRV_ALLOC_FAILED);
            // Set the new length
            *pStrLen = *pStrLen * 2 + 1;
        }
        // Copy this new character into it
        (*ppStr)[len] = (char)c;
        
        len++;
    }
    ASSERT(len > 0, GFMRV_TILEMAP_PARSING_ERROR);
    // Set the null terminator
    (*ppStr)[len] = '\0';
    
    // Try to remove the pushed position
    rv = gfmFile_clearLastPosStack(pFp);
    ASSERT(rv == GFMRV_OK, rv);
    isPosSet = 0;
    
    rv = GFMRV_OK;
__ret:
    // If the string wasn't parsed, return to the previous position
    if (isPosSet) {
        gfmFile_popPos(pFp);
    }
    
    return rv;
}

/**
 * Try to match a string in file
 * 
 * @param  pFp    The current file
 * @param  pStr   The string to be matched
 * @param  strLen The string's length
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_INTERNAL_ERROR, GFMRV_READ_ERROR
 */
gfmRV gfmParser_parseString(gfmFile *pFp, char *pStr, int strLen) {
    char c;
    gfmRV rv;
    int i, isPosSet;
    
    // Set default values
    isPosSet = 0;
    
    // Sanitize arguments
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(strLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Get the current position, to "backtrack" on error
    rv = gfmFile_pushPos(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    isPosSet = 1;
    
    // Loop through all characters on the string
    i = 0;
    while (i < strLen) {
        // Read the next character
        rv = gfmFile_readChar(&c, pFp);
        ASSERT_NR(rv == GFMRV_OK);
        // Check that the string was read
        ASSERT(pStr[i] == c, GFMRV_FALSE);
        
        i++;
    }
    // Check that the next character would be blank
    rv = gfmFile_readChar(&c, pFp);
    ASSERT_NR(rv == GFMRV_OK);
    ASSERT(gfmParser_isBlank(c), GFMRV_FALSE);
    rv = gfmFile_unreadChar(pFp);
    
    // Ignore all blank spaces so we are on the next token
    rv = gfmParser_ignoreBlank(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to remove the pushed position
    rv = gfmFile_clearLastPosStack(pFp);
    ASSERT(rv == GFMRV_OK, rv);
    isPosSet = 0;
    
    rv = GFMRV_TRUE;
__ret:
    // If the string wasn't parsed, return to the previous position
    if (isPosSet) {
        gfmFile_popPos(pFp);
    }
    
    return rv;
}

/**
 * Try to read a int from a file
 * 
 * @param  pVal The parsed integer
 * @param  pFp  The current file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *              GFMRV_READ_ERROR, GFMRV_TILEMAP_PARSING_ERROR
 */
gfmRV gfmParser_parseInt(int *pVal, gfmFile *pFp) {
    gfmRV rv;
    int isPosSet, num, signal;
    
    // Set default values
    isPosSet = 0;
    
    // Sanitize arguments
    ASSERT(pVal, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFp, GFMRV_ARGUMENTS_BAD);
    
    // Get the current position, to "backtrack" on error
    rv = gfmFile_pushPos(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    isPosSet = 1;
    
    // Read all characters until a blank space
    signal = 0;
    num = 0;
    while (1) {
        char c;
        
        // Read the next character
        rv = gfmFile_readChar(&c, pFp);
        ASSERT_NR(rv == GFMRV_OK);
        // If it's a blank char, stop parsing
        if (gfmParser_isBlank(c)) {
            break;
        }
        // If the signal isn't set, the character may be '-' to signal negative
        if (signal == 0 && num == 0 && c == '-') {
            signal = -1;
        }
        else if (c >= '0' && c <= '9') {
            // Otherwise, check that it's a valid char and accumulate it
            num = num * 10 + c - '0';
            // Set the number as positive, if it wasn't set yet
            if (signal == 0) {
                signal = 1;
            }
        }
        else {
            ASSERT(0, GFMRV_TILEMAP_PARSING_ERROR);
        }
    }
    // Check that a number was successfully read
    ASSERT(signal != 0, GFMRV_TILEMAP_PARSING_ERROR);
    
    // Ignore all blank spaces so we are on the next token
    rv = gfmParser_ignoreBlank(pFp);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to remove the pushed position
    rv = gfmFile_clearLastPosStack(pFp);
    ASSERT(rv == GFMRV_OK, rv);
    isPosSet = 0;
    
    // Set the return value
    *pVal = num * signal;
    rv = GFMRV_OK;
__ret:
    // If the string wasn't parsed, return to the previous position
    if (isPosSet) {
        gfmFile_popPos(pFp);
    }
    
    return rv;
}

