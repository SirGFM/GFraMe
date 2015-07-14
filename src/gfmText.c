/**
 * @file include/GFraMe/gfmText.h
 * 
 * Render-able text; Can be set on either screen or world space. Can also be
 * used to display a long message in a few lines, as well as animate the text
 * when it's being presented
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gfmText.h>

#include <stdlib.h>
#include <string.h>

/** The gfmText structure */
struct stGFMText {
    /** Character displayed this frame or '\0' */
    char lastChar;
    /** Text's horizontal position */
    int x;
    /** Text's vertical position */
    int y;
    /** Delay between characters */
    int charDelay;
    /** Delay between words (when a blank char comes after a non-blank one) */
    int wordDelay;
    /** Delay between blank characters and any other */
    int blankDelay;
    /** Current delay (starts at '(char|word)delay' and decreases to zero) */
    int curDelay;
    /** First non-NULL character on the current line */
    int lineStart;
    /** How many lines there currently are in the text */
    int lineCount;
    /** String's length */
    int strLen;
    /** How many character are being displayed */
    int curPos;
    /** Texts first displayed line */
    int curLine;
    /** How many character can be displayed in a single line */
    int maxWidth;
    /** How many lines can be displayed at the same time */
    int maxLines;
    /** First 'character tile' in the tilemap */
    int firstTile;
    /** Whether this text is world or screen bound */
    int worldBound;
    /** Tilemap with a bitmap font (starting at firstTile) */
    gfmSpriteset *pSset;
    /** The text content */
    gfmString *pStr;
};

/**
 * Get a new text 'object'
 * 
 * @param  ppCtx The text object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmText_getNew(gfmText **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmText*)malloc(sizeof(gfmText));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean the struct
    memset(*ppCtx, 0x0, sizeof(gfmText));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Releases a text object
 * 
 * @param  ppCtx The text object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAr
 */
gfmRV gfmText_free(gfmText **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the object
    gfmText_clean(*ppCtx);
    // Free its memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int delay, int bindToWorld, gfmSpriteset *pSset, int firstTile) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Initialize everything
    rv = gfmText_setPosition(pCtx, x, y);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_setDimensions(pCtx, maxWidth, maxLines);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_setTypingAnimation(pCtx, delay);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_setRenderSpace(pCtx, bindToWorld);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_setSpriteset(pCtx, pSset, firstTile);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Clear those two... just in case...
    pCtx->strLen = 0;
    pCtx->curPos = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean this 'text object'; All internal memory is freed so, to use it again,
 * it must be reinitialized
 * 
 * @param  pCtx The text
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_clean(gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove the previous string
    rv = gfmString_free(&(pCtx->pStr));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmText_setText(gfmText *pCtx, char *pStr, int len, int doCopy) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    
    // Alloc a new string, if there's none yet
    if (pCtx->pStr == 0) {
        rv = gfmString_getNew(&pCtx->pStr);
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Set the text
    rv = gfmString_init(pCtx->pStr, pStr, len, doCopy);
    ASSERT_NR(rv == GFMRV_OK);
    // Clean everything related to the string
    pCtx->lastChar = '\0';
    pCtx->lineStart = 0;
    pCtx->lineCount = 0;
    pCtx->curPos = 0;
    pCtx->curLine = 0;
    // TODO Remove trailing '\0' from length?
    pCtx->strLen = len;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the text's position
 * 
 * @param  pCtx The text
 * @param  x    Horizontal position
 * @param  y    Vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setPosition(gfmText *pCtx, int x, int y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the text position
    pCtx->x = x;
    pCtx->y = y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the text's dimensions
 * 
 * @param  pCtx     The text
 * @param  maxWidth How many character fit in a line
 * @param  maxLines How many lines can be displayed; The text will
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setDimensions(gfmText *pCtx, int maxWidth, int maxLines) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxLines > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the new dimensions
    pCtx->maxWidth = maxWidth;
    pCtx->maxLines = maxLines;
    
    // Normalize the text if it changes while a string is being animated
    if (pCtx->strLen) {
        rv = gfmText_normalize(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set how the text should be animated
 * 
 * @param  pCtx  The text
 * @param  delay How long it should take for each new character to appear;
 *               Anything greater than 0 activates this while anything
 *               less or equal to 0 makes the complete text appear at once
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setTypingAnimation(gfmText *pCtx, int delay) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // For now, set all delays the same way
    pCtx->charDelay = delay;
    pCtx->wordDelay = delay;
    pCtx->blankDelay = 0;
    
    // Finish the animation, if the delay is equal or less than zero
    if (delay <= 0 && pCtx->strLen > 0) {
        rv = gfmText_forceFinish(pCtx);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set how the text should be positioned and rendered
 * 
 * @param  pCtx        The text
 * @param  bindToWorld Whether it should be bounded to world space (i.e., move
 *                     with the camera and have a fixed position in the world)
 *                     or screen space (static position)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_setRenderSpace(gfmText *pCtx, int bindToWorld) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set what this text is bound to
    pCtx->worldBound = bindToWorld;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the text's spriteset
 * 
 * @param  pCtx        The text
 * @param  pSset       The spriteset
 * @param  firstTile   Index of the first character tile on the spriteset; Must
 *                     be in ASCII order and start at '!'
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmText_setSpriteset(gfmText *pCtx, gfmSpriteset *pSset, int firstTile) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(firstTile >= 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the spriteset and the first tile
    pCtx->pSset = pSset;
    pCtx->firstTile = firstTile;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

// TODO Getters

/**
 * Whether the text has finished animated
 * 
 * @param  pCtx The text
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_didFinish(gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check if the animation finished
    if (pCtx->curPos == pCtx->strLen)
        rv = GFMRV_TRUE;
    else
        rv = GFMRV_FALSE;
__ret:
    return rv;
}

/**
 * Forces a text to finish its animation; If there are multiple lines, it will
 * move to the last one
 * 
 * @param  pCtx The text
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NOT_SET
 */
gfmRV gfmText_forceFinish(gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if there's a string
    ASSERT(pCtx->strLen > 0, GFMRV_TEXT_NOT_SET);
    
    // Set the string to its max size
    pCtx->curPos = pCtx->strLen - 1;
    // Normalize the string
    rv = gfmText_normalize(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmText_getJustRendered(char *pChar, gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pChar, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that a character was displayed this/the previous frame
    ASSERT(pCtx->lastChar != '\0', GFMRV_TEXT_NO_CHAR);
    
    // Return the character
    *pChar = pCtx->lastChar;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line above
 * If the animation hasn't ended, this will return GFMRV_OPERATION_ACTIVE
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *               GFMRV_TEXT_NO_MORE_LINES
 */
gfmRV gfmText_moveLineUp(gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if the animation finished
    rv = gfmText_didFinish(pCtx);
    ASSERT(rv == GFMRV_TRUE, GFMRV_OPERATION_ACTIVE);
    
    // Check that there are hidden lines
    ASSERT(pCtx->curLine > 0, GFMRV_TEXT_NO_MORE_LINES);
    // Move the displayed area one line up
    pCtx->curLine--;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * If the text's content has more than 'maxLines' lines, moves the display
 * region one line bellow
 * If the animation hasn't ended, this will return GFMRV_OPERATION_ACTIVE
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *               GFMRV_TEXT_NO_MORE_LINES
 */
gfmRV gfmText_moveLineDown(gfmText *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if the animation finished
    rv = gfmText_didFinish(pCtx);
    ASSERT(rv == GFMRV_TRUE, GFMRV_OPERATION_ACTIVE);
    
    // Check that there are hidden lines
    ASSERT(pCtx->curLine + pCtx->maxLines - 1 < pCtx->lineCount,
            GFMRV_TEXT_NO_MORE_LINES);
    // Move the displayed area one line bellow
    pCtx->curLine++;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Normalizes a string up to its current position
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXT_NOT_SET
 */
gfmRV gfmText_normalize(gfmText *pCtx) {
    char *pStr;
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there's a string
    ASSERT(pCtx->strLen > 0, GFMRV_TEXT_NOT_SET);
    ASSERT(pCtx->pStr, GFMRV_TEXT_NOT_SET);
    
    // Retrieve the text's content
    rv = gfmString_getString(&pStr, pCtx->pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Normalize the current string
    i = 0;
    pCtx->lineStart = 0;
    pCtx->lineCount = 0;
    while (i < pCtx->curPos) {
        char c;
        
        // Get the current character
        c = pStr[i];
        
        // If we just got a line break, go to the next character
        if (c == '\n') {
            pCtx->lineCount++;
            // Skip all blank characters
            while (pStr[i + 1] == ' ') {
                i++;
            }
            // Set the line's starting position
            pCtx->lineStart = i + 1;
        }
        else if (i - pCtx->lineStart > pCtx->maxWidth) {
            int tmp;
            
            // Find the last blank character
            tmp = i - 1;
            while (pStr[tmp] != ' ') {
                tmp--;
            }
            // Set it to a linebreak
            pStr[tmp] = '\n';
            // Update the line count and line start
            pCtx->lineCount++;
            pCtx->lineStart = tmp + 1;
        }
        
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Updates the text animation; If it's not animated, does nothing
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_update(gfmText *pText, gfmCtx *pCtx) {
    char *pStr;
    gfmRV rv;
    int ms;
    
    // Sanitize arguments
    ASSERT(pText, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove the last rendered character here to avoid a bug on the last char
    pText->lastChar = '\0';
    
    // Check that there is a non-finished animation
    rv = gfmText_didFinish(pText);
    // If the animation is finished, exit succesfully
    ASSERT(rv == GFMRV_FALSE, GFMRV_OK);
    // Get time elapsed from the previous frames
    rv = gfm_getElapsedTime(&ms, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Retrieve the text's string
    rv = gfmString_getString(&pStr, pText->pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Decrease the current character delay
    if (pText->curDelay > 0) {
        pText->curDelay -= ms;
    }
    // Advance as many characters as necessary/possible
    while (pText->curDelay <= 0 && pText->curPos < pText->strLen) {
        char cur, next;
        
        // Get both the current character and the next
        cur = pStr[pText->curPos];
        // This needs no checks as the string is NULL terminated
        next = pStr[pText->curPos + 1];
        
        // Check if the current character was a valid one
        if (cur != ' ' && cur != '\n') {
            if (next == ' ' || next == '\n') {
                // If the next character if blank, use the word delay
                pText->curDelay += pText->wordDelay;
            }
            else {
                // Use the delay within words
                pText->curDelay += pText->charDelay;
            }
        }
        else {
            // Blank characters have no delay, by default
            pText->curDelay += pText->blankDelay;
        }
        
        // If we just got a linebreak, update the context accordingly
        if (pStr[pText->curPos] == '\n') {
            pText->lineCount++;
            pText->lineStart = pText->curPos + 1;
        }
        
        // Forces the line to start at a word
        while (pStr[pText->lineStart] == ' '){
            pText->lineStart++;
        }
        
        // Go to the next character
        pText->curPos++;
        
        // Check if the current line has gone over the line's width
        if (pText->curPos - pText->lineStart > pText->maxWidth) {
            int tmp;
            
            // If so, break on the start of the current word
            tmp = pText->curPos - 1;
            while (pStr[tmp] != ' ' && pStr[tmp] != '\n') {
                tmp--;
            }
            
            // Force this position to be a breakline
            pStr[tmp] = '\n';
            // Set the lineStart and increase the lines count
            pText->lineStart = tmp + 1;
            pText->lineCount++;
        }
        
        // Updates the 'chunk' being displayed
        if (pText->lineCount - pText->curLine >= pText->maxLines) {
            pText->curLine = pText->lineCount - pText->maxLines + 1;
        }
        
        // Get the character that will be displayed this frame
        pText->lastChar = pStr[pText->curPos];
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draws the text
 * 
 * @param  pCtx  The text
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmText_draw(gfmText *pText, gfmCtx *pCtx) {
    char *pStr;
    gfmRV rv;
    int i, height, lines, width, x, y;
    
    // Sanitize arguments
    ASSERT(pText, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // TODO Enable world rendering
    ASSERT(pText->worldBound == 0, GFMRV_FUNCTION_NOT_IMPLEMENTED);
    
    // Retrieve the spriteset dimensions
    rv = gfmSpriteset_getDimension(&width, &height, pText->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Retrieve the text's string
    rv = gfmString_getString(&pStr, pText->pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Find the current position on the text
    i = 0;
    lines = pText->curLine;
    while (lines > 0) {
        if (pStr[i] == '\n')
            lines--;
        i++;
    }
    
    // TODO Start batch rendering
    
    // Render every character
    lines = pText->maxLines;
    x = pText->x;
    y = pText->y;
    while (lines > 0 && i < pText->curPos) {
        if (pStr[i] != ' ' && pStr[i] != '\n') {
            int tile;
            
            // Render the character
            tile = pStr[i] - '!' + pText->firstTile;
            rv = gfm_drawTile(pCtx, pText->pSset, x, y, tile);
            ASSERT_NR(rv == GFMRV_OK);
        }
        // Update the next character's position
        if (pStr[i] == '\n') {
            lines--;
            x = pText->x;
            y += height;
        }
        else {
            x += width;
        }
        i++;
    }
    
    // TODO End batch rendering
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

