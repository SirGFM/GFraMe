/**
 * @file src/include/GFraMe_int/gfmHitbox.h
 *
 * Define a hitbox, which is also part of objects and may be (in some
 * circunstances) used interchangebly as such.
 */
#ifndef __INT_GFMHITBOX_H__
#define __INT_GFMHITBOX_H__

#include <stdint.h>

/** An area that may be used as a gfmObject to trigger events */
struct stGFMHitbox {
    /** pContext's type */
    uint32_t type;
    /** Real type of the object (so an object may distinguish from a hitbox) */
    uint32_t innerType;
    /** Points to more specific data (e.g., the object's sprite or the hitbox's
     * trigger info) */
    void *pContext;
    /** Hitbox's top-left corner position */
    int32_t x;
    int32_t y;
    /** Hitbox's dimensions (stores only half, though) */
    int16_t hw;
    int16_t hh;
};

#endif /* __INT_GFMHITBOX_H__ */

