/**
 * @file include/GFraMe/gfmCamera.h
 * 
 * Converts world-space to screen-space
 */
#ifndef __GFMCAMERA_STRUCT__
#define __GFMCAMERA_STRUCT__

/** 'Exports' the gfmCamera structure */
typedef struct stGFMCamera gfmCamera;

#endif /* __GFMCAMERA_STRUCT__ */

#ifndef __GFMCAMERA_H_
#define __GFMCAMERA_H_

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

/** 'Exportable' size of gfmCamera */
extern const int sizeofGFMCamera;

/**
 * Alloc a new camera
 * 
 * @param  ppCtx The allocated camera
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmCamera_getNew(gfmCamera **ppCtx);

/**
 * Free and clean up a camera
 * 
 * @param  ppCtx The camera
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmCamera_free(gfmCamera **ppCtx);

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
gfmRV gfmCamera_init(gfmCamera *pCam, gfmCtx *pCtx, int camWidth, int camHeight);

/**
 * Set the world's dimensions; If it's smaller than the screen, the camera will
 * center on the world
 * 
 * @param  pCtx        The game's context
 * @param  worldWidth  The world's width
 * @param  worldHeight The world's height
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmCamera_setWorldDimensions(gfmCamera *pCtx, int worldWidth, int worldHeight);

/**
 * Set the deadzone (i.e., zone that, after moving out, will move the camera)
 * 
 * @param  pCtx   The camera
 * @param  x      The deadzone horizontal position (in screen)
 * @param  y      The deadzone vertical position (in screen)
 * @param  width  The deadzone width
 * @param  height The deadzone height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED,
 *                GFMRV_CAMERA_INVALID_WIDTH, GFMRV_CAMERA_INVALID_HEIGHT
 */
gfmRV gfmCamera_setDeadzone(gfmCamera *pCtx, int x, int y, int width, int height);

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
gfmRV gfmCamera_centerAtPoint(gfmCamera *pCtx, int x, int y);

//gfmRV gfmCamera_centerAtObject(gfmCamera *pCtx, gfmObject *pObj);

/**
 * Converts a point in screen space to world space
 * 
 * @param  pX   The position to be converted
 * @param  pY   The position to be converted
 * @param  pCtx The camera
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_screenToWorld(int *pX, int *pY, gfmCamera *pCtx);

/**
 * Sets the camera position ignoring the world's dimensions.
 *
 * @param pCtx The camera
 * @param x    The horizontal position
 * @param y    The vertical position
 * @return     GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_setPositionUnrestricted(gfmCamera *pCtx, int x, int y);

/**
 * Sets the camera position, ensuring its within the world's dimensions.
 *
 * @param pCtx The camera
 * @param x    The horizontal position
 * @param y    The vertical position
 * @return     GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_setPosition(gfmCamera *pCtx, int x, int y);

/**
 * Get the camera's current position
 * 
 * @param  pX   The current horizontal position
 * @param  pY   The current vertical position
 * @param  pCtx The camera
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmCamera_getPosition(int *pX, int *pY, gfmCamera *pCtx);

/**
 * Get the camera's dimension
 * 
 * @param  pWidth  The camera's width
 * @param  pHeigth The camera's height
 * @param  pCtx    The camera
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfmCamera_getDimensions(int *pWidth, int *pHeight, gfmCamera *pCtx);

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
gfmRV gfmCamera_isSpriteInside(gfmCamera *pCtx, gfmSprite *pSpr);

#endif /* __GFMCAMERA_H_ */

