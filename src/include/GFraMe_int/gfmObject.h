/**
 * @file src/include/GFraMe_int/gfmObject.h
 *
 * Non-export functions for gfmObjects.
 */
#ifndef __GFRAME_INT_GFMOBJECT_H__
#define __GFRAME_INT_GFMOBJECT_H__

/**
 * Get an object's collision boundary as if continous collision was enabled for
 * it.
 *
 * Should only be internally called, therefore it skips some checks...
 *
 * @param  [out]pX      Object's position
 * @param  [out]pY      Object's position
 * @param  [out]pWidth  Object's dimensions
 * @param  [out]pHeight Object's dimensions
 * @param  [ in]pCtx    The object
 */
void _gfmObject_getContinousCollisionBoundary(int *pX, int *pY, int *pWidth
       , int *pHeight, gfmObject *pCtx);

#endif /* __GFRAME_INT_GFMOBJECT_H__ */

