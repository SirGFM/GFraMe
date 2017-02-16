/**
 * @file src/include/GFraMe_int/gfmHitbox.h
 *
 * Define a hitbox, which is also part of objects and may be (in some
 * circunstances) used interchangebly as such.
 */
#ifndef __INT_GFMHITBOX_H__
#define __INT_GFMHITBOX_H__

#include <GFraMe/gfmTypes.h>
#include <stdint.h>

/** Type of a gfmType_hitbox. It could be anything as longs as it were different
 * from gfmType_object */
#define gfmType_hitbox gfmType_reserved_2

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

/**
 * Expand a previously alloc'ed list of hitboxes, without destroying the
 * previous one.
 *
 * @param  [ in]ppCtx    The alloc'ed list
 * @param  [ in]oldCount How many hitboxes had been alloc'ed
 * @param  [ in]newCount How many hitboxes should be alloc'ed
 */
gfmRV gfmHitbox_expandList(gfmHitbox **ppCtx, int oldCount, int newCount);

#endif /* __INT_GFMHITBOX_H__ */

