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
 * Set the text's content; The string WILL BE MODIFIED as needed, but it's
 * length won't be modified; Therefore, it's a users choice if the string will
 * be copied or not;
 * 
 * The string must be NULL terminated; so the user must take this care if he
 * doesn't copy the string
 * 
 * @param  pCtx   The text
 * @param  pStr   The string to be displayed
 * @param  len    The string's length
 * @param  doCopy Whether the string should be copied or only its pointer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setText(gfmText *pCtx, char *pStr, int len, int doCopy);

/**
 * Set the text's content; The string WILL BE MODIFIED as needed, but it's
 * length won't be modified; Therefore, it's a users choice if the string will
 * be copied or not;
 * 
 * The string must be NULL terminated; so the user must take this care if he
 * doesn't copy the string
 * 
 * @param  pCtx   The text
 * @param  pStr   The string to be displayed (in a static buffer)
 * @param  doCopy Whether the string should be copied or only its pointer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
#define gfmText_setTextStatic(pCtx, pStr, doCopy) \
        gfmText_setText(pCtx, pStr, sizeof(pStr) - 1, doCopy)

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

/**
 * Set the text's spriteset
 * 
 * @param  pCtx        The text
 * @param  pSset       The spriteset
 * @param  firstTile   Index of the first character tile on the spriteset; Must
 *                     be in ASCII order and start at '!'
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmText_setSpriteset(gfmText *pCtx, gfmSpriteset *pSset, int firstTile);

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
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NOT_SET
 */
gfmRV gfmText_forceFinish(gfmText *pCtx);

/**
 * If a character was displayed this frame*, return it; If more than one
 * character was displayed, returns the last character;
 * This can be useful to play a SFX whenever a 'non-null' character is displayed
 * 
 * * "this frame" means "since the previous gfmText_update"
 * 
 * @param  pChar The character
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NO_CHAR
 */
gfmRV gfmText_getJustRendered(char *pChar, gfmText *pCtx);

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line above
 * If the animation hasn't ended, this will return GFMRV_OPERATION_ACTIVE
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *               GFMRV_TEXT_NO_MORE_LINES
 */
gfmRV gfmText_moveLineUp(gfmText *pCtx);

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line bellow
 * If the animation hasn't ended, this will return GFMRV_OPERATION_ACTIVE
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *               GFMRV_TEXT_NO_MORE_LINES
 */
gfmRV gfmText_moveLineDown(gfmText *pCtx);

/**
 * Normalizes a string up to its current position
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NOT_SET
 */
gfmRV gfmText_normalize(gfmText *pCtx);

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

