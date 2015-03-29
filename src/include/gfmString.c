/**
 * @file src/include/GFraMe_int/gfmString.h
 * 
 * Defines a basic string structure
 */
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmString.h>

struct stGFMString {
    /** The string itself */
    char *self;
    /** The string's length */
    int len;
    /** Whether the string should be automatically deallocated */
    int mustDealloc;
};

/**
 * Copies a string from a static buffer
 * 
 * @param ppStr The created gfmString
 * @param string A statically allocated string (i.e., char var[] = "...")
 * @param len String's length
 * @param doCopy Whether the string's content or address will be stored
 */
gfmRV gfmString_init(gfmString **ppStr, char *string, int len, int doCopy) {
}

