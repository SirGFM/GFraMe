/**
 * @file src/include/GFraMe_int/gfmString.h
 * 
 * Defines a basic string structure
 */
#ifndef __GFMSTRING_STRUCT__
#define __GFMSTRING_STRUCT__

/** 'Exports' the gfmString structure */
typedef struct stGFMString gfmString;

#endif /* __GFMSTRING_STRUCT__ */

#ifndef __GFMSTRING_H__
#define __GFMSTRING_H__

#include <GFraMe/gfmError.h>

/** 'Exportable' size of gfmString */
extern const int sizeofGFMString;

/**
 * Alloc a new gfmString
 * 
 * @param  ppStr The gfmString
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmString_getNew(gfmString **ppStr);

/**
 * Free up a gfmString's memory
 * 
 * @param  ppStr The gfmString
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmString_free(gfmString **ppStr);

/**
 * Initializes a string from a static buffer
 * 
 * @param  pStr   The created gfmString
 * @param  string A statically allocated string (i.e., char var[] = "...")
 * @param  doCopy Whether the string's content or address will be stored
 */
#define gfmString_initStatic(pStr, string, doCopy) \
    gfmString_init(pStr, string, sizeof(string)-1, doCopy)

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
 * @param  string A string
 * @param  len    The string's length
 * @param  doCopy Whether the string's content (non-zero) or address (zero) will
 *                be stored
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmString_init(gfmString *pStr, char *string, int len, int doCopy);

/**
 * Check if a string needs to be extended and do it
 * 
 * @param  pStr The created gfmString
 * @param  len  The new desired length
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_STRING_WASNT_COPIED
 */
gfmRV gfmString_setMinimumLength(gfmString *pStr, int len);

/**
 * Concatenate a static string to a another one
 * 
 * NOTE: Can only be done if the string was initialized as a copy!
 * 
 * @param  pStr   The created gfmString
 * @param  string A statically allocated string (i.e., char var[] = "...")
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_STRING_WASNT_COPIED, GFMRV_STRING_TOO_SMALL
 */
#define gfmString_concatStatic(pStr, string) \
    gfmString_concat(pStr, string, sizeof(string)-1)

/**
 * Concatenate a string to another one
 * 
 * NOTE: Can only be done if the string was initialized as a copy!
 * 
 * @param  pStr   The created gfmString
 * @param  string A string
 * @param  len    The string's length
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_STRING_WASNT_COPIED, GFMRV_STRING_TOO_SMALL
 */
gfmRV gfmString_concat(gfmString *pStr, char *string, int len);

/**
 * Insert a static string into another one, at a random position
 * 
 * NOTE: Can only be done if the string was initialized as a copy!
 * 
 * @param  pStr   The created gfmString
 * @param  string A statically allocated string (i.e., char var[] = "...")
 * @param  pos    Position, on the original string, where it should be inserted
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_STRING_WASNT_COPIED
 */
#define gfmString_insertAtStatic(pStr, string, pos) \
    gfmString_insertAt(pStr, string, sizeof(string)-1, pos)

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
gfmRV gfmString_insertAt(gfmString *pStr, char *string, int len, int pos);

/**
 * Insert a decimal number into a string at a random position, and make that the
 * end of the string
 * 
 * @param  pStr The gfmString
 * @param  num  The number
 * @param  pos  Position, on the original string, where it should be inserted
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_STRING_WASNT_COPIED
 */
gfmRV gfmString_insertNumberAt(gfmString *pStr, int num, int pos);

/**
 * Get the string's content, as a NULL terminated char*
 * 
 * @param  ppStr The retrieved string
 * @param  pStr  The string
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_STRING_NOT_INITIALIZED
 */
gfmRV gfmString_getString(char **ppStr, gfmString *pStr);

/**
 * Set the string's length
 * 
 * @param  pStr The string
 * @param  len  The new length
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_STRING_NOT_INITIALIZED,
 *              GFMRV_ALLOC_FAILED
 */
gfmRV gfmString_setLength(gfmString *pStr, int len);

/**
 * Get the string's length
 * 
 * @param  pLen The string's length
 * @param  pStr  The string
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_STRING_NOT_INITIALIZED
 */
gfmRV gfmString_getLength(int *pLen, gfmString *pStr);

/**
 * Clear up previously allocated resources
 * 
 * @param  pStr   The created gfmString
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmString_clear(gfmString *pStr);

#endif /* __INT_GFMSTRING__ */

