/**
 * @file include/GFraMe/gfmCamera.h
 * 
 * Converts world-space to screen-space
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

#include <stdlib.h>
#include <string.h>

/** camera struct */
struct stGFMCamera {
    /** Camera's position into the world */
    int x;
    /** Camera's position into the world */
    int y;
    /** Camera's width */
    int viewWidth;
    /** Camera's height */
    int viewHeight;
    /** World's width */
    int worldWidth;
    /** World's height */
    int worldHeight;
    /** Dead zone's center position */
    int deadX;
    /** Dead zone's center position */
    int deadY;
    /** Dead zone's half width */
    int deadWidth;
    /** Dead zone's half height */
    int deadHeight;
};

/** 'Exportable' size of gfmCamera */
const int sizeofGFMCamera = (int)sizeof(gfmCamera);

/**
 * Alloc a new camera
 * 
 * @param  ppCtx The allocated camera
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmCamera_getNew(gfmCamera **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and zero the camera
    *ppCtx = (gfmCamera*)malloc(sizeof(gfmCamera));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmCamera));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free and clean up a camera
 * 
 * @param  ppCtx The camera
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmCamera_free(gfmCamera **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the camera
 * 
 * @param  pCam      The camera
 * @param  pCtx      The game's context
 * @param  camWidth  The camera's view width
 * @param  camHeight The camera's view height
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_INVALID_WIDTH,
 *                   GFMRV_CAMERA_INVALID_HEIGHT
 */
gfmRV gfmCamera_init(gfmCamera *pCam, gfmCtx *pCtx, int camWidth, int camHeight) {
    gfmRV rv;
    int bbufWidth, bbufHeight;
    
    // Sanitize arguments
    ASSERT(pCam, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(camWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(camHeight > 0, GFMRV_ARGUMENTS_BAD);
    
    // Get the backbuffer's dimension
    rv = gfm_getBackbufferDimensions(&bbufWidth, &bbufHeight, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Check that the dimensions are valid
    ASSERT(camWidth <= bbufWidth, GFMRV_CAMERA_INVALID_WIDTH);
    ASSERT(camHeight <= bbufHeight, GFMRV_CAMERA_INVALID_HEIGHT);
    
    // Set the camera's parameters
    pCam->x = 0;
    pCam->y = 0;
    pCam->viewWidth = camWidth;
    pCam->viewHeight = camHeight;
    pCam->worldWidth = camWidth;
    pCam->worldHeight = camHeight;
    pCam->deadX = 0;
    pCam->deadY = 0;
    pCam->deadWidth = camWidth;
    pCam->deadHeight = camHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the world's dimensions; If it's smaller than the screen, the camera will
 * center on the world
 * 
 * @param  pCtx        The game's context
 * @param  worldWidth  The world's width
 * @param  worldHeight The world's height
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_setWorldDimensions(gfmCamera *pCtx, int worldWidth, int worldHeight) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(worldWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(worldHeight > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it's initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    ASSERT(pCtx->viewHeight > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Set the world's dimensions
    pCtx->worldWidth = worldWidth;
    pCtx->worldHeight = worldHeight;
    
    // Check if the camera requires centering or adjusting to the position
    if (pCtx->worldWidth < pCtx->viewWidth)
        pCtx->x = (pCtx->viewWidth - pCtx->worldWidth) / 2;
    else if (pCtx->x + pCtx->viewWidth > pCtx->worldWidth)
        pCtx->x = pCtx->worldWidth - pCtx->viewWidth;
    else if (pCtx->x < 0)
        pCtx->x = 0;
    
    if (pCtx->worldHeight < pCtx->viewHeight)
        pCtx->y = (pCtx->viewHeight - pCtx->worldHeight) / 2;
    else if (pCtx->y + pCtx->viewHeight > pCtx->worldHeight)
        pCtx->y = pCtx->worldHeight - pCtx->viewHeight;
    else if (pCtx->y < 0)
        pCtx->y = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the deadzone (i.e., zone that, after moving out, will move the camera);
 * Internally, the deadzone is stored as an AABB
 * 
 * @param  pCtx   The camera
 * @param  x      The deadzone horizontal position (in screen)
 * @param  y      The deadzone vertical position (in screen)
 * @param  width  The deadzone width
 * @param  height The deadzone height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED,
 *                GFMRV_CAMERA_INVALID_WIDTH, GFMRV_CAMERA_INVALID_HEIGHT
 */
gfmRV gfmCamera_setDeadzone(gfmCamera *pCtx, int x, int y, int width, int height) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(x >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(y >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    // Check that the dimensions are valid
    ASSERT(x + width <= pCtx->viewWidth, GFMRV_CAMERA_INVALID_WIDTH);
    ASSERT(x + height <= pCtx->viewHeight, GFMRV_CAMERA_INVALID_HEIGHT);
    
    // Set the deadzone
    pCtx->deadWidth = width / 2;
    pCtx->deadHeight = height / 2;
    pCtx->deadX = x + pCtx->deadWidth;
    pCtx->deadY = y + pCtx->deadHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Center the camera at a point; If the point is inside the deadzone, it stays
 * still
 * 
 * @param  pCtx The camera
 * @param  x    Horizontal position of the point
 * @param  y    Vertical position of the point
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED,
 *              GFMRV_CAMERA_MOVED, GFMRV_CAMERA_DIDNT_MOVE
 */
gfmRV gfmCamera_centerAtPoint(gfmCamera *pCtx, int x, int y) {
    gfmRV rv;
    int dx, dy;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Default return
    rv = GFMRV_CAMERA_DIDNT_MOVE;
    
    // Check the distance to the camera
    dx = x - pCtx->x - pCtx->deadX;
    dy = y - pCtx->y - pCtx->deadY;
    
    // 'Center' it at the position (vertical axis)
    if (dy > pCtx->deadHeight || dy < -pCtx->deadHeight) {
        pCtx->y = y - pCtx->deadY;
        if (dy > 0)
            pCtx->y -= pCtx->deadHeight;
        else if (dy < 0)
            pCtx->y += pCtx->deadHeight;
        rv = GFMRV_CAMERA_MOVED;
    }
    // Check that it's in-bounds (vertical axis)
    if (pCtx->y < 0) {
        pCtx->y = 0;
        rv = GFMRV_CAMERA_DIDNT_MOVE;
    }
    else if (pCtx->y + pCtx->viewHeight > pCtx->worldHeight) {
        pCtx->y = pCtx->worldHeight - pCtx->viewHeight;
        rv = GFMRV_CAMERA_DIDNT_MOVE;
    }
    
    // 'Center' it at the position (horizontal axis)
    if (dx > pCtx->deadWidth || dx < -pCtx->deadWidth) {
        pCtx->x = x - pCtx->deadX;
        if (dx > 0)
            pCtx->x -= pCtx->deadWidth;
        else if (dx < 0)
            pCtx->x += pCtx->deadWidth;
        rv = GFMRV_CAMERA_MOVED;
    }
    // Check that it's in-bounds (horizontal axis)
    if (pCtx->x < 0) {
        pCtx->x = 0;
        pCtx->x = pCtx->worldWidth - pCtx->viewWidth;
        rv = GFMRV_CAMERA_DIDNT_MOVE;
    }
    else if (pCtx->x + pCtx->viewWidth > pCtx->worldWidth) {
        pCtx->x = pCtx->worldWidth - pCtx->viewWidth;
        rv = GFMRV_CAMERA_DIDNT_MOVE;
    }
    
    // If not modified, the default return was set as GFMRV_CAMERA_DIDNT_MOVE
__ret:
    return rv;
}

/**
 * Converts a point in screen space to world space
 * 
 * @param  pX   The position to be converted
 * @param  pY   The position to be converted
 * @param  pCtx The camera
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_screenToWorld(int *pX, int *pY, gfmCamera *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    // Check that it's within bounds
    ASSERT(*pX >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pX <= pCtx->viewWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pY >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pY <= pCtx->viewHeight, GFMRV_ARGUMENTS_BAD);
    
    // Adjust the position
    *pX += pCtx->x;
    *pY += pCtx->y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the camera's current position
 * 
 * @param  pX   The current horizontal position
 * @param  pY   The current vertical position
 * @param  pCtx The camera
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmCamera_getPosition(int *pX, int *pY, gfmCamera *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the current position
    *pX = pCtx->x;
    *pY = pCtx->y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the camera's dimension
 * 
 * @param  pWidth  The camera's width
 * @param  pHeigth The camera's height
 * @param  pCtx    The camera
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_getDimensions(int *pWidth, int *pHeight, gfmCamera *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Get the current position
    *pWidth = pCtx->viewWidth;
    *pHeight = pCtx->viewHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if an object is inside the camera
 * 
 * @param  pCtx The camera
 * @param  pObj The object
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_isObjectInside(gfmCamera *pCtx, gfmObject *pObj);

/**
 * Check if an sprite is inside the camera
 * 
 * @param  pCtx The camera
 * @param  pSpr The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_isSpriteInside(gfmCamera *pCtx, gfmSprite *pSpr) {
    gfmSpriteset *pSset;
    gfmRV rv;
    int offX, offY, tileWidth, tileHeight, x, y;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->viewWidth > 0, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Get the needed params
    rv = gfmSprite_getSpriteset(&pSset, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_getOffset(&offX, &offY, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_getPosition(&x, &y, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pSset);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Fix the sprite's position
    x += offX;
    y += offY;
    
    // Check that it's inside the camera
    ASSERT(x <= pCtx->x + pCtx->viewWidth, GFMRV_FALSE);
    ASSERT(x + tileWidth >= pCtx->x, GFMRV_FALSE);
    ASSERT(y <= pCtx->y + pCtx->viewHeight, GFMRV_FALSE);
    ASSERT(y + tileHeight >= pCtx->y, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

