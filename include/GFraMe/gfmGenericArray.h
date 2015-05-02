/**
 * @file include/GFraMe/gfmGenericArray.h
 * 
 * A generic array that can be easily expanded (reallocated); built upon
 * macros to be strongly typed
 */
#ifndef __GFMGENERICARRAY_H__
#define __GFMGENERICARRAY_H__

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <stdlib.h>
#include <string.h>

/**
 * Create a expandable buffer of a given type
 * 
 * @param TYPE The type
 */
#define gfmGenArr_define(TYPE) \
    typedef struct { \
        TYPE **arr; \
        int len; \
        int used; \
    } TYPE##_genericArray; \

/**
 * Create a variable on the given type
 * 
 * @param TYPE    The type
 * @param VARNAME The variable's name
 */
#define gfmGenArr_var(TYPE, VARNAME) \
    TYPE##_genericArray VARNAME

/**
 * Zero out a buffer
 * 
 * @param TYPE The type
 */
#define gfmGenArr_zero(buffer) \
    do { \
        buffer.arr = 0; \
        buffer.len = 0; \
        buffer.used = 0; \
    } while (0)

/**
 * Set the minimum size of a buffer, expanding it if necessary
 * 
 * @param TYPE     The type
 * @param buffer   The buffer
 * @param SIZE     Minimum size for the buffer
 * @param INI_FUNC Function to be called for newly instantiated stuff (e.g., if
 *                 type is an pointer and must be allocated). Make sure its only
 *                 parameter is a pointer to TYPE. Also, it must return an int
 *                 and, on success, 0
 */
#define gfmGenArr_setMinSize(TYPE, buffer, SIZE, INI_FUNC) \
    do { \
        if (buffer.len < (SIZE)) { \
            int i = buffer.len; \
            /* Alloc the new buffer */ \
            TYPE **tmp = (TYPE**)malloc(sizeof(TYPE*) * (SIZE)); \
            ASSERT(tmp, GFMRV_ALLOC_FAILED); \
            /* Clean it up */ \
            memset(tmp, 0x0, sizeof(TYPE*) * (SIZE)); \
            /* Copy the old buffer and release its memory */ \
            if (buffer.arr) { \
                memcpy(tmp, buffer.arr, sizeof(TYPE*) * buffer.len); \
                free(buffer.arr); \
            } \
            /* Initialize every new node */ \
            while (i < (SIZE)) { \
                rv = INI_FUNC(&tmp[i]); \
                ASSERT_NR(rv == 0); \
                i++; \
            } \
            /* Update both the buffer and its size */ \
            buffer.arr = tmp; \
            buffer.len = SIZE; \
        } \
    } while (0)

/**
 * Clean up all memory used by this type's buffer
 * 
 * @param buffer     The buffer
 * @param CLEAN_FUNC Function to be called per buffer object. Its only parameter
 *                   must be a pointer to TYPE
 */
#define gfmGenArr_clean(buffer, CLEAN_FUNC) \
    do { \
        /* Check that the array isn't NULL */ \
        if (buffer.arr) { \
            int i = 0; \
            /* Dealloc every object on the buffer */\
            while (i < buffer.len) { \
                CLEAN_FUNC(&(buffer.arr[i])); \
                i++; \
            } \
            /* Release the buffer's memory */\
            free(buffer.arr); \
            gfmGenArr_zero(buffer); \
        } \
    } while (0)

/**
 * Get the reference to the next object (and expand the buffer as necessary)
 * 
 * @param TYPE     The type
 * @param buffer   The buffer
 * @param INC      By how much should the buffer expand, if necessary
 * @param REF      Variable's name where the reference will be returned. Must be
 *                 a pointer to TYPE
 * @param INI_FUNC Function to be called for newly instantiated stuff (e.g., if
 *                 type is an pointer and must be allocated). Make sure its only
 *                 parameter is a pointer to TYPE
 */
#define gfmGenArr_getNextRef(TYPE, buffer, INC, REF, INI_FUNC) \
    do { \
        /* Check if the buffer must be expanded */ \
        if (buffer.used >= buffer.len) { \
            gfmGenArr_setMinSize(TYPE, buffer, buffer.len + INC, INI_FUNC); \
        } \
        /* Get a valid new reference */ \
        REF = buffer.arr[buffer.used]; \
    } while (0)

/**
 * Recycle a reference to an object (but expand the buffer, if necessary)
 * 
 * @param TYPE          The type
 * @param buffer        The buffer
 * @param INC           By how much should the buffer expand, if necessary
 * @param REF           Variable's name where the reference will be returned.
 *                      Must be a pointer to TYPE
 * @param IS_VALID_FUNC Function to validate a referece. It must taken a pointer
 *                      to TYPE and, if valid, must return 1
 * @param INI_FUNC      Function to be called for newly instantiated stuff
 *                      (e.g., if type is an pointer and must be allocated).
 *                      Make sure its only parameter is a pointer to TYPE
 */
#define gfmGenArr_recycle(TYPE, buffer, INC, REF, IS_VALID_FUNC, INI_FUNC) \
    do { \
        /* Search for a valid object */ \
        int i = 0, found = 0; \
        while (i < buffer.used) { \
            if (IS_VALID_FUNC(buffer.arr[i])) { \
                REF = buffer.arr[i]; \
                found = 1; \
                break; \
            } \
            i++; \
        } \
        /* If coudln't find, get one from the buffer */ \
        if (!found) {\
            /* Check if the buffer must be expanded */ \
            if (buffer.used >= buffer.len) { \
                gfmGenArr_setMinSize(TYPE, buffer, buffer.len + INC, INI_FUNC); \
            } \
            /* Get a valid new reference */ \
            REF = buffer.arr[buffer.used]; \
            BUF_PUSH(buffer); \
        } \
    } while (0)

/**
 * Validate a previous "gfmGenArr_getNextRef" by increasing the amount of used
 * items
 * 
 * @param buffer The buffer
 */
#define gfmGenArr_push(buffer) \
    buffer.used++

/**
 * Get how many objects are in use
 * 
 * @param buffer The buffer
 */
#define gfmGenArr_getUsed(buffer) \
    buffer.used

/**
 * Get a object from the buffer
 * 
 * @param buffer The buffer
 * @param NUM    The object's index (no check is made!!)
 */
#define gfmGenArry_getObject(buffer, NUM) \
    buffer.arr[NUM]

/**
 * Call something in every object int the buffer
 * 
 * @param buffer The buffer
 * @param CALL_  Initial portition of the calling function. It must expect a
 *               following parameter of type TYPE
 * @param ...    Any other required params
 */
#define gfmGenArr_callAll(buffer, CALL, ...) \
    do { \
        int i = 0; \
        while (i < buffer.used) { \
            CALL(buffer.arr[i], ##__VA_ARGS__) ; \
            i++; \
        } \
    } while (0)

/**
 * Call something in every object int the buffer and handle error
 * 
 * @param buffer The buffer
 * @param CALL   Initial portition of the calling function. It must expect a
 *               following parameter of type TYPE (and return GFMRV_OK)
 * @param ...    Any other required params
 */
#define gfmGenArr_callAllRV(buffer, CALL, ...) \
    do { \
        int i = 0; \
        while (i < buffer.used) { \
            rv = CALL(buffer.arr[i], ##__VA_ARGS__) ; \
            ASSERT_NR(rv == GFMRV_OK); \
            i++; \
        } \
    } while (0)

#endif /* __GFMGENERICARRAY_H__ */

