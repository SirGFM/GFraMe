/**
 * @file include/GFraMe/gfmText.h
 * 
 * Render-able text; Can be set on either screen or world space. Can also be
 * used to display a long message in a few lines, as well as animate the text
 * when it's being presented
 */
#ifndef __GFMTEXT_STRUCT__
#define __GFMTEXT_STRUCT__

/** 'Export' the gfmText structure */
typedef struct stGFMText gfmText;

#endif /* __GFMTEXT_STRUCT__ */

#ifndef __GFMTEXT_H__
#define __GFMTEXT_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>

/**
 * Get a new text 'object'
 * 
 * @param  ppCtx The text object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmText_getNew(gfmText **ppCtx);

/**
 * Releases a text object
 * 
 * @param  ppCtx The text object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAr
 */
gfmRV gfmText_free(gfmText **ppCtx);

/**
 * (Re)Initialize a text
 * 
 * @param  pCtx        The text
 * @param  x           Horizontal position
 * @param  y           Vertical position
 * @param  maxWidth    How many character fit in a line
 * @param  maxLines    How many lines can be displayed; The text will
 *                     automatically scroll; If there's no delay, it will wait
 *                     for a command to move the text!
 * @param  delay       How long it should take for each new character to appear;
 *                     Anything greater than 0 activates this while anything
 *                     less or equal to 0 makes the complete text appear at once
 * @param  bindToWorld Whether it should be bounded to world space (i.e., move
 *                     with the camera and have a fixed position in the world)
 *                     or screen space (static position)
 * @param  pSset       The spriteset
 * @param  firstTile   Index of the first character tile on the spriteset; Must
 *                     be in ASCII order and start at '!'
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmText_init(gfmText *pCtx, int x, int y, int maxWidth, int maxLines,
        int delay, int bindToWorld, gfmSpriteset *pSset, int firstTile);

/**
 * Clean this 'text object'; All internal memory is freed so, to use it again,
 * it must be reinitialized
 * 
 * @param  pCtx The text
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_clean(gfmText *pCtx);

/**
 * Set the text's content; It will be copied into a gfmString(?)
 * 
 * @param  pCtx The text
 * @param  pStr The string to be displayed
 * @param  len  The string's length
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setText(gfmText *pCtx, char *pStr, int len);

/**
 * Set the text's contents, from a static buffer
 * 
 * @param  pCtx The text
 * @param  pStr The string to be displayed
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
#define gfmText_setTextState(pCtx, pStr) \
        gfmText_setText(pCtx, pStr, sizeof(pStr) - 1)

/**
 * Set the text's position
 * 
 * @param  pCtx The text
 * @param  x    Horizontal position
 * @param  y    Vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setPosition(gfmText *pCtx, int x, int y);

/**
 * Set the text's dimensions
 * 
 * @param  pCtx     The text
 * @param  maxWidth How many character fit in a line
 * @param  maxLines How many lines can be displayed; The text will
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setDimensions(gfmText *pCtx, int maxWidth, int maxLines);

/**
 * Set how the text should be animated
 * 
 * @param  pCtx  The text
 * @param  delay How long it should take for each new character to appear;
 *               Anything greater than 0 activates this while anything
 *               less or equal to 0 makes the complete text appear at once
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setTypingAnimation(gfmText *pCtx, int delay);

/**
 * Set how the text should be positioned and rendered
 * 
 * @param  pCtx        The text
 * @param  bindToWorld Whether it should be bounded to world space (i.e., move
 *                     with the camera and have a fixed position in the world)
 *                     or screen space (static position)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setRenderSpace(gfmText *pCtx, int bindToWorld);

// TODO Getters

/**
 * Whether the text has finished animated
 * 
 * @param  pCtx The text
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_didFinish(gfmText *pCtx);

/**
 * Forces a text to finish its animation; If there are multiple lines, it will
 * move to the last one
 * 
 * @param  pCtx The text
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_forceFinish(gfmText *pCtx);

/**
 * If a character was displayed this frame, return it; If more than one
 * character was displayed, returns the last character;
 * This can be useful to play a SFX whenever a 'non-null' character is displayed
 * 
 * @param  pChar The character
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NO_CHAR
 */
gfmRV gfmText_getJustRendered(char *pChar, gfmText *pCtx);

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line above
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_moveLineUp(gfmText *pCtx);

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line bellow
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_moveLineDown(gfmText *pCtx);

/**
 * Updates the text animation; If it's not animated, does nothing
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_update(gfmText *pText, gfmCtx *pCtx);

/**
 * Draws the text
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_draw(gfmText *pText, gfmCtx *pCtx);

#endif /* __GFMTEXT_H__ */

