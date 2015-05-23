/**
 * @file src/gfmSprite.c
 * 
 * Represent a retangular 'object' that can be rendered to the screen; It has
 * its own gfmObject to handle the physics
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTypes.h>

#include <stdlib.h>
#include <string.h>

/** The gfmSprite structure */
struct stGFMSprite {
    /** The sprite's physical object */
    gfmObject *pObject;
    /** The sprite's 'child-class' (e.g., a player struct) */
    void *pChild;
    /** The sprite child's type */
    int childType;
    /** Horizontal offset between the object's top-left corner and the tile's */
    int offsetX;
    /** Vertical offset between the object's top-left corner and the tile's */
    int offsetY;
    /** The sprite's spriteset */
    gfmSpriteset *pSset;
    /** Current frame (i.e., tile) from the spriteset */
    int frame;
    /** Whether the sprite is flipped */
    int isFlipped;
    // TODO Animation...
};

/** Size of gfmSprite */
const int sizeofGFMSprite = (int)sizeof(gfmSprite);

/**
 * Alloc a new gfmSprite
 * 
 * @param  ppCtx The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSprite_getNew(gfmSprite **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the sprite
    *ppCtx = (gfmSprite*)malloc(sizeof(gfmSprite));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Clean everything
    memset(*ppCtx, 0x0, sizeof(gfmSprite));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated gfmSprite
 * 
 * @param  ppCtx The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_free(gfmSprite **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the sprite
    rv = gfmSprite_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a sprite given its top-left position and its dimensions
 * 
 * NOTE: Every one of the sprite's attributes are cleared out, on calling this
 * function
 * 
 * @param  pCtx   The sprite
 * @param  x      The sprite's horizontal position
 * @param  y      The sprite's vertical position
 * @param  width  The sprite's width
 * @param  height The sprite's height
 * @param  pSset  The sprite's spriteset
 * @param  offX   Tile's horizontal offset from the object's position
 * @param  offY   Tile's vertical offset from the object's position
 * @param  pChild The sprite's "sub-class" (e.g., a gfmSprite)
 * @param  type   The type of the sprite's "sub-class"
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSprite_init(gfmSprite *pCtx, int x, int y, int width, int height,
        gfmSpriteset *pSset, int offX, int offY, void *pChild, int type) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    
    // If there's no object, alloc it
    if (!pCtx->pObject) {
        rv = gfmObject_getNew(&(pCtx->pObject));
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // Initialize the object
    rv = gfmObject_init(pCtx->pObject, x, y, width, height, (void*)pCtx,
            gfmType_sprite);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the spriteset and offset
    pCtx->pSset = pSset;
    pCtx->offsetX = offX;
    pCtx->offsetY = offY;
    
    // TODO animations
    
    // Set the child
    pCtx->pChild = pChild;
    pCtx->childType = type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear the sprite
 * 
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSprite_clean(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Dealloc the object, if any
    gfmObject_free(&(pCtx->pObject));
    // TODO animation...
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the sprite's dimensions
 * 
 * @param  pCtx   The sprite
 * @param  width  The sprite's width
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setDimensions(gfmSprite *pCtx, int width, int height) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setDimensions(pCtx->pObject, width, height);
__ret:
    return rv;
}

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx  The sprite
 * @param  width The sprite's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalDimension(gfmSprite *pCtx, int width) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalDimension(pCtx->pObject, width);
__ret:
    return rv;
}

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx   The sprite
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalDimension(gfmSprite *pCtx, int height) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalDimension(pCtx->pObject, height);
__ret:
    return rv;
}

/**
 * Get the sprite's dimensions
 * 
 * @param  pWidth  The sprite's width
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDimensions(int *pWidth, int *pHeight, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getDimensions(pWidth, pHeight, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's width
 * 
 * @param  pWidth The sprite's width
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getWidth(int *pWidth, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getWidth(pWidth, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's height
 * 
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHeight(int *pHeight, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHeight(pHeight, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set a sprite's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  x    The horizontal position
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setPosition(gfmSprite *pCtx, int x, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setPosition(pCtx->pObject, x, y);
__ret:
    return rv;
}

/**
 * Set a sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalPosition(gfmSprite *pCtx, int x) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalPosition(pCtx->pObject, x);
__ret:
    return rv;
}

/**
 * Set a sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalPosition(gfmSprite *pCtx, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalPosition(pCtx->pObject, y);
__ret:
    return rv;
}

/**
 * Get the sprite's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getPosition(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getPosition(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalPosition(int *pX, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalPosition(pX, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalPosition(int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalPosition(pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's central position
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getCenter(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCenter(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's central position on the previous frame
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getLastCenter(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getLastCenter(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVelocity(gfmSprite *pCtx, double vx, double vy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVelocity(pCtx->pObject, vx, vy);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalVelocity(gfmSprite *pCtx, double vx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalVelocity(pCtx->pObject, vx);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalVelocity(gfmSprite *pCtx, double vy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalVelocity(pCtx->pObject, vy);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVelocity(double *pVx, double *pVy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVelocity(pVx, pVy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalVelocity(double *pVx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalVelocity(pVx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalVelocity(double *pVy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalVelocity(pVy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setAcceleration(gfmSprite *pCtx, double ax, double ay) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setAcceleration(pCtx->pObject, ax, ay);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalAcceleration(gfmSprite *pCtx, double ax) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalAcceleration(pCtx->pObject, ax);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalAcceleration(gfmSprite *pCtx, double ay) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalAcceleration(pCtx->pObject, ay);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getAcceleration(double *pAx, double *pAy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getAcceleration(pAx, pAy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalAcceleration(double *pAx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalAcceleration(pAx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalAcceleration(double *pAy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalAcceleration(pAy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setDrag(gfmSprite *pCtx, double dx, double dy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setDrag(pCtx->pObject, dx, dy);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setHorizontalDrag(gfmSprite *pCtx, double dx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalDrag(pCtx->pObject, dx);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setVerticalDrag(gfmSprite *pCtx, double dy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalDrag(pCtx->pObject, dy);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDrag(double *pDx, double *pDy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' functionSSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = gfmObject_getDrag(pDx, pDy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalDrag(double *pDx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDrag(pDx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalDrag(double *pDy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDrag(pDy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Force this sprite to stand immovable on collision
 * 
 * NOTE: An sprite can move through its physics even if fixed!
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setFixed(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setFixed(pCtx->pObject);
__ret:
    return rv;
}
/**
 * Allow this sprite to move on collision
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setMovable(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setMovable(pCtx->pObject);
__ret:
    return rv;
}

/**
 * Update the sprite; Its last collision status is cleared and the sprite's
 * properties are integrated using the Euler method
 * 
 * @param  pSpr The sprite
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_update(gfmSprite *pSpr, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_update(pSpr->pObject, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // TODO Update animation
__ret:
    return rv;
}

/**
 * Get the distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getDistance(int *pDx, int *pDy, gfmSprite *pSelf,
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getDistance(pDx, pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the horizontal distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getHorizontalDistance(int *pDx, gfmSprite *pSelf,
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDistance(pDx, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the vertical distance between two sprites' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getVerticalDistance(int *pDy, gfmSprite *pSelf,
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDistance(pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the horizontal distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getHorizontalDistanced(double *pDx, gfmSprite *pSelf,
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDistanced(pDx, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the vertical distance between two sprites' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getVerticalDistanced(double *pDy, gfmSprite *pSelf,
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDistanced(pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Check if a give point is inside the sprite
 * 
 * @param  pCtx The sprite
 * @param  x    The point's horizontal position
 * @param  y    The point's vertical position
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_isPointInside(gfmSprite *pCtx, int x, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_isPointInside(pCtx->pObject, x, y);
__ret:
    return rv;
}

/**
 * Check if two sprites are overlaping
 * 
 * NOTE: It fails to detect if an sprite was inside another one and is leaving
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_isOverlaping(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_isOverlaping(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Collide two sprites
 * 
 * NOTE: It fails to detect if an sprite was inside another one and is leaving
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED, GFMRV_OBJECTS_CANT_COLLIDE
 */
gfmRV gfmSprite_collide(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_collide(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Separate two sprite in the X axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateHorizontal(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_separateHorizontal(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Separate two sprite in the Y axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateVertical(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_separateVertical(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getLastCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getLastCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the collision/overlap flags from the last 'gfmSprite_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCurrentCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCurrentCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's (i.e., image/tile) offset from the object's hitbox
 * 
 * @param  pCtx The sprite
 * @param  offX Tile's horizontal offset from the object's position
 * @param  offY Tile's vertical offset from the object's position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setOffset(gfmSprite *pCtx, int offX, int offY) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Set the offset
    pCtx->offsetX = offX;
    pCtx->offsetY = offY;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the sprite's (i.e., image/tile) offset from the object's hitbox
 * 
 * @param  pOffX Tile's horizontal offset from the object's position
 * @param  pOffY Tile's vertical offset from the object's position
 * @param  pCtx  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getOffset(int *pOffX, int *pOffY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOffX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOffY, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Get the offset
    *pOffX = pCtx->offsetX;
    *pOffY = pCtx->offsetY;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the sprite's sub-class (e.g, a player, an enemy, etc)
 * 
 * NOTE: Even if the sprite has no sub-class, this function's ppChild param
 * mustn't be NULL
 * 
 * @param  ppChild The sub-class struct
 * @param  pType   The sub-class type
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getChild(void **ppChild, int *pType, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppChild, GFMRV_ARGUMENTS_BAD);
    ASSERT(pType, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Get the child
    *ppChild = pCtx->pChild;
    *pType = pCtx->childType;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the sprite's super-class (i.e., a gfmObject)
 * 
 * @param  ppObj The object
 * @param  pCtx  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getObject(gfmObject **ppObj, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppObj, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Get the object
    *ppObj = pCtx->pObject;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the sprite's spriteset
 * 
 * @param  pCtx  The sprite
 * @param  pSset The sprite's spriteset
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setSpriteset(gfmSprite *pCtx, gfmSpriteset *pSset) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Set the spriteset
    pCtx->pSset = pSset;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the sprite's spriteset
 * 
 * @param  ppSset The sprite's spriteset
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getSpriteset(gfmSpriteset **ppSset, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppSset, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Get the spriteset
    *ppSset = pCtx->pSset;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the current frame
 * 
 * @param  pCtx  The sprite
 * @param  frame The frame
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setFrame(gfmSprite *pCtx, int frame) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(frame > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Set the current frame
    pCtx->frame = frame;
    // TODO stop animation?
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the current frame
 * 
 * @param  pFrame The frame
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getFrame(int *pFrame, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFrame, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Get the current frame
    *pFrame = pCtx->frame;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Render the sprite on the screen (if it's inside it)
 * 
 * @param  pSpr The sprite
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_draw(gfmSprite *pSpr, gfmCtx *pCtx) {
    gfmRV rv;
    int camX, camY, x, y;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pSpr->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Check if the sprite is inside the camera
    rv = gfm_isSpriteInsideCamera(pCtx, pSpr);
    ASSERT_NR(rv == GFMRV_TRUE);
    
    // Get camera's dimension
    rv = gfm_getCameraPosition(&camX, &camY, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the object's position
    rv = gfmSprite_getPosition(&x, &y, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Calculate the sprite's actual position
    x = x + pSpr->offsetX - camX;
    y = y + pSpr->offsetY - camY;
    
    // Render the tile to the screen
    rv = gfm_drawTile(pCtx, pSpr->pSset, x, y, pSpr->frame);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

