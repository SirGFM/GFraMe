/**
 * @file src/gfmSprite.c
 * 
 * Represent a retangular 'object' that can be rendered to the screen; It has
 * its own gfmObject to handle the physics
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAnimation.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTypes.h>

#include <stdlib.h>
#include <string.h>

/** Define an array for animation */
gfmGenArr_define(gfmAnimation);

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
    /** Sprite's animations */
    gfmGenArr_var(gfmAnimation, pAnimations);
    /** The playing animation, if any */
    gfmAnimation *pCurAnim;
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
    
    // Reset its direction
    rv = gfmSprite_setDirection(pCtx, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Reset the previous animations
    gfmGenArr_callAllRV(pCtx->pAnimations, gfmAnimation_clean);
    gfmGenArr_reset(pCtx->pAnimations);
    
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
    
    // Free the animations
    gfmGenArr_clean(pCtx->pAnimations, gfmAnimation_free);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Reset the sprite's object (clearing it's velocity, acceleration, collision
 * flags, etc.); The object is kept at its current place
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_resetObject(gfmSprite *pCtx) {
    gfmRV rv;
    int height, width, x, y;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the sprite's current position and dimensions
    rv = gfmSprite_getDimensions(&width, &height, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_getPosition(&x, &y, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Reset it
    rv = gfmObject_init(pCtx->pObject, x, y, width, height, (void*)pCtx,
            gfmType_sprite);
    ASSERT_NR(rv == GFMRV_OK);
    
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
    
    // Update animation
    if (pSpr->pCurAnim) {
        rv = gfmAnimation_update(pSpr->pCurAnim, pCtx);
        ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_ANIMATION_ALREADY_FINISHED);
        
        rv = gfmAnimation_didJustChangeFrame(pSpr->pCurAnim);
        if (rv == GFMRV_TRUE) {
            // Update the sprite's frame
            rv = gfmAnimation_getFrame(&(pSpr->frame), pSpr->pCurAnim);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
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
 * Set the current frame; Any playing animation will be stopped!
 * 
 * @param  pCtx  The sprite
 * @param  frame The frame
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setFrame(gfmSprite *pCtx, int frame) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(frame >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    // Set the current frame
    pCtx->frame = frame;
    pCtx->pCurAnim = 0;
    
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
 * Set the direction the sprite is facing (either forward or backward)
 * 
 * @param  pCtx      The sprite
 * @param  isFlipped Whether it's flipped
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setDirection(gfmSprite *pCtx, int isFlipped) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    pCtx->isFlipped = isFlipped;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the direction the sprite is facing (either forward or backward)
 * 
 * @param  pFlipped Whether it's flipped
 * @param  pCtx     The sprite
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getDirection(int *pFlipped, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFlipped, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pObject, GFMRV_SPRITE_NOT_INITIALIZED);
    
    *pFlipped = pCtx->isFlipped;
    
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
    //ASSERT_NR(rv == GFMRV_TRUE);
    if (rv != GFMRV_TRUE) {
        rv = GFMRV_OK;
        goto __ret;
    }
    //ASSERT(rv == GFMRV_TRUE, GFMRV_OK);
    
    // Get camera's dimension
    rv = gfm_getCameraPosition(&camX, &camY, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the object's position
    rv = gfmSprite_getPosition(&x, &y, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Calculate the sprite's actual position
    if (pSpr->isFlipped) {
        int th, tw, width;
        
        // Get the tile's dimension (so we can flip it)
        rv = gfmSpriteset_getDimension(&tw, &th, pSpr->pSset);
        ASSERT_NR(rv == GFMRV_OK);
        rv = gfmObject_getWidth(&width, pSpr->pObject);
        ASSERT_NR(rv == GFMRV_OK);
        
        x = x + (-tw + width - pSpr->offsetX) - camX;
    }
    else {
        x = x + pSpr->offsetX - camX;
    }
    y = y + pSpr->offsetY - camY;
    
    // Render the tile to the screen
    rv = gfm_drawTile(pCtx, pSpr->pSset, x, y, pSpr->frame, pSpr->isFlipped);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add a animation to the sprite
 * 
 * @param  pIndex    Animation's index on this sprite
 * @param  pCtx      The sprite
 * @param  pData     Animation's frames
 * @param  numFrames Number of frame on animation
 * @param  fps       Animation's framerate
 * @param  doLoop    Whether the animation should loop or not
 */
gfmRV gfmSprite_addAnimation(int *pIndex, gfmSprite *pCtx, int *pData,
        int numFrames, int fps, int doLoop) {
    gfmAnimation *pAnim;
    gfmRV rv;
    int inc;
    
    // Sanitize arguments
    ASSERT(pIndex, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    
    // Get the next reference
    inc = 1;
    gfmGenArr_getNextRef(gfmAnimation, pCtx->pAnimations, inc, pAnim,
            gfmAnimation_getNew);
    
    // Initialize it
    rv = gfmAnimation_init(pAnim, pData, numFrames, fps, doLoop);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Push the animation and get its index
    *pIndex = gfmGenArr_getUsed(pCtx->pAnimations);
    gfmGenArr_push(pCtx->pAnimations);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add a batch of animations to the sprite;
 * The array must be organized in the following format:
 * 
 * pData[0] = anim_0.numFrames
 * pData[1] = anim_0.fps
 * pData[2] = anim_0.doLoop
 * pData[3] = anim_0.frame_0
 * ...
 * pData[3 + anim_0.numFrames - 1] = anim_0.lastFrame
 * pData[3 + anim_0.numFrames] = anim_1.numFrames
 * ...
 * 
 * And so on.
 * 
 * @param  pCtx    The sprite
 * @param  pData   The batch of animations
 * @param  dataLen How many integers there are in pData
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSprite_addAnimations(gfmSprite *pCtx, int *pData, int dataLen) {
    gfmRV rv;
    int *pAnimData, doLoop, fps, i, index, numFrames;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    // A Batch with a single two-frames animation would have 5 ints
    ASSERT(dataLen >= 5, GFMRV_ARGUMENTS_BAD);
    
    // Add every animation
    i = 0;
    while (i < dataLen) {
        // Get the animation parameters
        numFrames = pData[i + 0];
        fps       = pData[i + 1];
        doLoop    = pData[i + 2];
        pAnimData = pData + i + 3;
        // Add the animation
        rv = gfmSprite_addAnimation(&index, pCtx, pAnimData, numFrames, fps, doLoop);
        ASSERT_NR(rv == GFMRV_OK);
        // Go to the next animation
        i += numFrames + 3;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Resets the currently playing animation
 * 
 * @param  pCtx  The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_resetAnimation(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a animation playing
    ASSERT(pCtx->pCurAnim, GFMRV_NO_ANIMATION_PLAYING);
    
    rv = gfmAnimation_reset(pCtx->pCurAnim);
__ret:
    return rv;
}

/**
 * Resets and plays an animation
 * 
 * @param  pCtx  The sprite
 * @param  index Index of the animation to be played
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfmSprite_playAnimation(gfmSprite *pCtx, int index) {
    gfmAnimation *pAnim;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that it's a valid index
    ASSERT(index < gfmGenArr_getUsed(pCtx->pAnimations), GFMRV_INVALID_INDEX);
    
    // Get the animation
    pAnim = gfmGenArr_getObject(pCtx->pAnimations, index);
    
    // Don't re-play the animation
    if (pAnim == pCtx->pCurAnim) {
        rv = GFMRV_OK;
        goto __ret;
    }
    //ASSERT(pAnim != pCtx->pCurAnim, GFMRV_OK);
    
    // Reset it
    rv = gfmAnimation_reset(pAnim);
    ASSERT_NR(rv == GFMRV_OK);
    // And play it
    pCtx->pCurAnim = pAnim;
    // Frame must be updated (otherwise, it'll only be on the next frame change)
    rv = gfmAnimation_getFrame(&(pCtx->frame), pCtx->pCurAnim);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Returns whether the animation have already looped
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationLoop(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a animation playing
    ASSERT(pCtx->pCurAnim, GFMRV_NO_ANIMATION_PLAYING);
    
    rv = gfmAnimation_didLoop(pCtx->pCurAnim);
__ret:
    return rv;
}

/**
 * Returns whether the animation just looped
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationJustLoop(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a animation playing
    ASSERT(pCtx->pCurAnim, GFMRV_NO_ANIMATION_PLAYING);
    
    rv = gfmAnimation_didJustLoop(pCtx->pCurAnim);
__ret:
    return rv;
}

/**
 * Returns whether the animation just change the frame
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationJustChangeFrame(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a animation playing
    ASSERT(pCtx->pCurAnim, GFMRV_NO_ANIMATION_PLAYING);
    
    rv = gfmAnimation_didJustChangeFrame(pCtx->pCurAnim);
__ret:
    return rv;
}

/**
 * Returns whether the animation finished running
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationFinish(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a animation playing
    ASSERT(pCtx->pCurAnim, GFMRV_NO_ANIMATION_PLAYING);
    
    rv = gfmAnimation_didFinish(pCtx->pCurAnim);
__ret:
    return rv;
}

