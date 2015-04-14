/**
 * @file src/include/GFraMe_int/gfmString.h
 * 
 * Defines a basic string structure
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>

struct stGFMString {
    /** The string itself */
    char *self;
    /** The string's length */
    int len;
    /** Length of the buffer */
    int bufLen;
    /** Whether the string should be automatically deallocated */
    int mustDealloc;
};

/** 'Exportable' size of gfmString */
const int sizeofGFMString = sizeof(struct stGFMString);

/**
 * Alloc a new gfmString
 * 
 * @param  ppStr The gfmString
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmString_getNew(gfmString **ppStr) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppStr), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the structure
    *ppStr = (gfmString*)malloc(sizeofGFMString);
    ASSERT(*ppStr, GFMRV_ALLOC_FAILED);
    
    // Initialize the structure
    (*ppStr)->self = 0;
    (*ppStr)->len = 0;
    (*ppStr)->bufLen = 0;
    (*ppStr)->doDealloc = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free up a gfmString's memory
 * 
 * @param  ppStr The gfmString
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmString_free(gfmString **ppStr) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppStr, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the structure
    gfmStrinc_clear(*ppStr);
    
    // Free up the memory
    free(*ppStr);
    *ppStr = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initializes a string
 * 
 * If 'doCopy' was set as zero and another call is made with 'doCopy' as
 * non-zero, the previous string will be deallocated; If any number of string
 * are initialized without copying, no extra memory will be allocated; If a
 * copied string is initialized over another copied string, the buffer will be 
 * expanded (i.e., realloc'ed) as necessary
 * 
 * @param  pStr   The created gfmString
 * @param  string A statically allocated string (i.e., char var[] = "...")
 * @param  len    The string's length
 * @param  doCopy Whether the string's content (non-zero) or address (zero) will
 *                be stored
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmString_init(gfmString *pStr, char *string, int len, int doCopy) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(string, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the string and simply use the pointer
    if (!doCopy) {
        if (pStr->doDealloc)
            gfmString_clear(pStr);
        // Copy the string's address
        pStr->self = string;
        pStr->bufLen = 0;
    }
    // Check if the string must be extended
    else if (doCopy) {
        int i;
        
        // Extend the string's length, if necessary
        if (!pStr->doDealloc) {
            pStr->self = 0;
            pStr->doDealloc = 1;
        }
        rv = gfmString_setMinimumLength(pStr, len + 1);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Copy the string's contents
        i = 0;
        while (i < len) {
            pStr->self[i] = string[i];
            i++;
        }
        // Append a '\0' to the end of the string
        pStr->self[i] = '\0';
    }
    
    // Set whether the string must be dealloc'ed and its length
    pStr->doDealloc = doCopy;
    pStr->len = len;
    
    rv = GFMRV_OK;
__ret:
    if (rv == GFMRV_ALLOC_FAILED)
        pStr->bufLen = 0;
    
    return rv;
}

/**
 * Check if a string needs to be extended and do it
 * 
 * @param  pStr The created gfmString
 * @param  len  The new desired length
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_STRING_WASNT_COPIED
 */
gfmRV gfmString_setMinimumLength(gfmString *pStr, int len) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the string was alloc
    ASSERT(pStr->doDealloc, GFMRV_STRING_WASNT_COPIED);
    // Check that the string must be expanded
    ASSERT(pStr->bufLen < len, GFMRV_OK);
    
    // Extend the string as necessary
    if (pStr->self && pStr->bufLen < len)
        pStr->self = (char*)realloc(pStr->self, len);
    else if (!pStr->self)
        pStr->self = (char*)malloc(len);
    ASSERT(pStr->self, GFMRV_ALLOC_FAILED);
    
    pStr->bufLen = len;
    rv = GFMRV_OK;
__ret:
    if (rv == GFMRV_ALLOC_FAILED)
        pStr->bufLen = 0;
    
    return rv;
}

/**
 * Concatenate a string to another one;
 * 
 * NOTE: Can only be done if the string was initialized as a copy!
 * 
 * @param  pStr   The created gfmString
 * @param  string A string
 * @param  len    The string's length
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_STRING_WASNT_COPIED
 */
gfmRV gfmString_concat(gfmString *pStr, char *string, int len) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    
    // Insert the string at the end of the previous
    rv = gfmString_insertAt(pStr, string, len, pStr->len);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Insert an 'EOS'
    pStr->self[pStr->len] = '\0';
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Insert a string into another one, at a random position
 * 
 * NOTE: Can only be done if the string was initialized as a copy!
 * 
 * @param  pStr   The created gfmString
 * @param  string A string
 * @param  len    The string's length
 * @param  pos    Position, on the original string, where it should be inserted
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_STRING_WASNT_COPIED
 */
gfmRV gfmString_insertAt(gfmString *pStr, char *string, int len, int pos) {
    gfmRV rv;
    int i;
    
    // Sanitize the arguments
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pos >= 0 && pos < pStr->len, GFMRV_ARGUMENTS_BAD);
    // Check that the string was alloc
    ASSERT(pStr->doDealloc, GFMRV_STRING_WASNT_COPIED);
    
    // Extend the string, as necessary
    gfmString_setMinimumLength(pStr, pStr->len + len + 1);
    // Concatenate both strings
    i = 0;
    while (i < len) {
        pStr->self[pos + i] = string[i];
        i++;
    }
    // Set the string's new length
    pStr->len = pos + i;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the string's content, as a NULL terminated char*
 * 
 * @param  ppStr The retrieved string
 * @param  pStr  The string
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmString_getString(char **ppStr, gfmString *pStr) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppStr), GFMRV_ARGUMENTS_BAD);
    ASSERT(pstr, GFMRV_ARGUMENTS_BAD);
    
    // Check that the string was initialized
    ASSERT(pStr->self, GFMRV_STRING_NOT_INITIALIZED);
    // Get the string's pointer
    *ppStr = pStr->self;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the string's length
 * 
 * @param  pLen The string's length
 * @param  pStr  The string
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_STRING_NOT_INITIALIZED
 */
gfmRV gfmString_getLength(int *pLen, gfmString *pStr) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pLen, GFMRV_ARGUMENTS_BAD);
    ASSERT(pstr, GFMRV_ARGUMENTS_BAD);
    // Check that the string was initialized
    ASSERT(pStr->self, GFMRV_STRING_NOT_INITIALIZED);
    // Get the string's length
    *pLen = pStr->len;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear up previously allocated resources
 * 
 * @param  pStr   The created gfmString
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmString_clear(gfmString *pStr) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    
    // Free the previous string
    if (pStr->doDealloc)
        free(pStr->self);
    // Clean up variables
    pStr->self = 0;
    pStr->bufLen = 0;
    pStr->len = 0;
    pStr->doDealloc = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

