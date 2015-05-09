/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration, drag
 * and current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#ifndef __GFMOBJECT_STRUCT__
#define __GFMOBJECT_STRUCT__

/** 'Exports' the gfmObject structure */
typedef struct stGFMObject gfmObject;

#endif  /* __GFMOBJECT_STRUCT__ */

#ifndef __GFMOBJECT_H__
#define __GFMOBJECT_H__

#include <GFraMe/gfmError.h>

/** 'Exportable' size of gfmObject */
extern const int sizeofGFMObject;

gfmRV gfmObject_getNew(gfmObject **ppCtx);
gfmRV gfmObject_free(gfmObject **ppCtx);
gfmRV gfmObject_init(gfmObject *pCtx, int x, int y, int w, int h);
gfmRV gfmObject_clean(gfmObject *pCtx);
gfmRV gfmObject_setDimensions(gfmObject *pCtx, int width, int height);
gfmRV gfmObject_setHorizontalDimension(gfmObject *pCtx, int width);
gfmRV gfmObject_setVerticalDimension(gfmObject *pCtx, int height);
gfmRV gfmObject_getDimensions(int *pWidth, int *pHeight, gfmObject *pCtx);
gfmRV gfmObject_getHorizontalDimension(int *pWidth, gfmObject *pCtx);
gfmRV gfmObject_getVerticalDimension(int *pHeight, gfmObject *pCtx);
gfmRV gfmObject_getCenter(int *pX, int *pY, gfmObject *pCtx);
gfmRV gfmObject_setPosition(gfmObject *pCtx, int x, int y);
gfmRV gfmObject_setHorizontalPosition(gfmObject *pCtx, int x);
gfmRV gfmObject_setVerticalPosition(gfmObject *pCtx, int y);
gfmRV gfmObject_getPosition(int *pX, int *pY, gfmObject *pCtx);
gfmRV gfmObject_getHorizontalPosition(int *pX, gfmObject *pCtx);
gfmRV gfmObject_getVerticalPosition(int *pY, gfmObject *pCtx);
gfmRV gfmObject_setVelocity(gfmObject *pCtx, int vx, int vy);
gfmRV gfmObject_setHorizontalVelocity(gfmObject *pCtx, int vx);
gfmRV gfmObject_setVerticalVelocity(gfmObject *pCtx, int vy);
gfmRV gfmObject_getVelocity(int *pVx, int *pVy, gfmObject *pCtx);
gfmRV gfmObject_getHorizontalVelocity(int *pVx, gfmObject *pCtx);
gfmRV gfmObject_getVerticalVelocity(int *pVy, gfmObject *pCtx);
gfmRV gfmObject_setAcceleration(gfmObject *pCtx, int ax, int ay);
gfmRV gfmObject_setHorizontalAcceleration(gfmObject *pCtx, int ax);
gfmRV gfmObject_setVerticalAcceleration(gfmObject *pCtx, int ay);
gfmRV gfmObject_getAcceleration(int *pAx, int *pAy, gfmObject *pCtx);
gfmRV gfmObject_getHorizontalAcceleration(int *pAx, gfmObject *pCtx);
gfmRV gfmObject_getVerticalAcceleration(int *pAy, gfmObject *pCtx);
gfmRV gfmObject_setDrag(gfmObject *pCtx, int dx, int dy);
gfmRV gfmObject_setHorizontalDrag(gfmObject *pCtx, int dx);
gfmRV gfmObject_setVerticalDrag(gfmObject *pCtx, int dy);
gfmRV gfmObject_getDrag(int *pDx, int *pDy, gfmObject *pCtx);
gfmRV gfmObject_getHorizontalDrag(int *pDx, gfmObject *pCtx);
gfmRV gfmObject_getVerticalDrag(int *pDy, gfmObject *pCtx);
gfmRV gfmObject_update(gfmObject *pCtx, int ms);
gfmRV gfmObject_getChild(void **ppChild, int *pType, gfmObject *pCtx);
gfmRV gfmObject_isPointInside(gfmObject *pCtx, int x, int y);
gfmRV gfmObject_isOverlaping(gfmObject *pSelf, gfmObject *pOther);
gfmRV gfmObject_separateHorizontal(gfmObject *pSelf, gfmObject *pOther);
gfmRV gfmObject_separateVertical(gfmObject *pSelf, gfmObject *pOther);
gfmRV gfmObject_getCollisionDirection(/* ???, */gfmObject *pCtx);
gfmRV gfmObject_getLastCollisionDirection(/* ???, */gfmObject *pCtx);
gfmRV gfmObject_getDistance(int *pDx, int *pDy, gfmObject *pSelf,
        gfmObject *pOther);

#endif  /* __GFMOBJECT_H__ */

