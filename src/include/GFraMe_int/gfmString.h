/**
 * @file src/include/GFraMe_int/gfmString.h
 * 
 * Defines a basic string structure
 */
#ifndef __INT_GFMSTRING__
#define __INT_GFMSTRING__

#include <GFraMe/gfmError.h>

typedef struct stGFMString gfmString;

/**
 * Copies a string from a static buffer
 * 
 * @param ppStr The created gfmString
 * @param string A statically allocated string (i.e., char var[] = "...")
 * @param doCopy Whether the string's content or address will be stored
 */
#define gfmString_initStatic(ppStr, string, doCopy) \
    gfmString_init(ppStr, string, sizeof(string), doCopy)

/**
 * Copies a string from a static buffer
 * 
 * @param ppStr The created gfmString
 * @param string A statically allocated string (i.e., char var[] = "...")
 * @param doCopy Whether the string's content or address will be stored
 */
gfmRV gfmString_init(gfmString **ppStr, char *string, int len, int doCopy);

#endif /* __INT_GFMSTRING__ */

