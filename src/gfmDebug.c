#if defined(DEBUG)
/**
 * @file src/gfmDebug.c
 */

#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>

#include <GFraMe_int/gframe.h>
#include <GFraMe_int/gfmCtx_struct.h>
#include <GFraMe_int/gfmDebug.h>
#include <GFraMe_int/gfmDebug_font.h>

#include <stdarg.h>
#include <string.h>

/**
 * Initialize the debug context
 *
 * @param  [ in]pCtx The lib's main context
 */
gfmRV gfmDebug_init(gfmCtx *pCtx) {
    gfmRV rv;
    int tex;

    if (pCtx == 0) {
        return GFMRV_LOG_NOT_INITIALIZED;
    }
    else if (pCtx->pDebugSset) {
        return GFMRV_OK;
    }

    rv = _gfm_loadBinTexture(&tex, pCtx, _fontData, _fontWidth, _fontHeight);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_createSpritesetCached(&pCtx->pDebugSset, pCtx, tex, _fontTileWidth,
            _fontTileHeight);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Print a string to the screen.
 *
 * @param  [ in]pCtx  The framework's main context
 * @param  [ in]x     The horizontal position in screen space
 * @param  [ in]y     The vertical position in screen space
 * @param  [ in]pText The text to be printed
 * @param  [ in]len   The length of the text
 * @return            Number of printed characters
 */
int gfmDebug_printText(gfmCtx *pCtx, int x, int y, char *pText, int len) {
    int i, _x;

    if (pCtx == 0 || pCtx->pDebugSset) {
        return 0;
    }

    i = 0;
    _x = x;
    while (i < len) {
        char c = pText[i];

        if (c == '\n') {
            _x = x;
            y += _fontTileHeight;
        }
        else {
            if (c != ' ') {
                gfm_drawTile(pCtx, pCtx->pDebugSset, _x, y, c - '!', 0);
            }
            _x += _fontTileWidth;
        }
        i++;
    }

    return len;
}

/**
 * Print an integer number in decimal base to the screen.
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]val  The number to be printed
 * @param  [ in]len  Minimum number of digits (padded with 0s)
 * @return           Number of printed characters
 */
int gfmDebug_printInt(gfmCtx *pCtx, int x, int y, int val, int len) {
    if (pCtx == 0 || pCtx->pDebugSset) {
        return 0;
    }

    if (val < 0) {
        /* Negative number, the lazy way */
        gfm_drawTile(pCtx, pCtx->pDebugSset, x, y, '-' - '!', 0);
        return gfmDebug_printInt(pCtx, x + _fontTileWidth, y, -val, len) + 1;
    }
    else {
        int numLen;
        /* Enough characters for 32 bits */
        char buf[10];

        /* Ensure all limits are respected */
        if (len > sizeof(buf)) {
            len = sizeof(buf);
        }
        val = val & 0xFFFFFFFF;
        memset(buf, '0' - '!', sizeof(buf));

        /* Fill the buffer with printable characters */
        numLen = sizeof(buf);
        while (val > 0) {
            int digit = val % 10;
            val /= 10;
            buf[numLen - 1] = (char)digit + '0' - '!';
            numLen--;
        }
        numLen = sizeof(buf) - numLen;

        /* Print the number and add padding as necessary */
        if (numLen < len) {
            gfmDebug_printText(pCtx, x, y, buf + sizeof(buf) - len, len);
            return len;
        }
        else {
            gfmDebug_printText(pCtx, x, y, buf + sizeof(buf) - numLen, numLen);
            return numLen;
        }
    }
}

/**
 * Print an integer number in hexadecimal base number to the screen.
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]hexa The number to be printed
 * @param  [ in]len  Minimum number of digits (padded with 0s)
 * @return           Number of printed characters
 */
int gfmDebug_printHexa(gfmCtx *pCtx, int x, int y, int hexa, int len) {
    /* Enough characters for 32 bits */
    char buf[8];
    int numLen;

    if (pCtx == 0 || pCtx->pDebugSset) {
        return 0;
    }

    /* Ensure all limits are respected */
    if (len > sizeof(buf)) {
        len = sizeof(buf);
    }
    hexa = hexa & 0xFFFFFFFF;
    memset(buf, '0' - '!', sizeof(buf));

    gfm_drawTile(pCtx, pCtx->pDebugSset, x, y, '0' - '!', 0);
    gfm_drawTile(pCtx, pCtx->pDebugSset, x + _fontTileWidth, y, 'X' - '!', 0);
    x += _fontTileWidth * 2;

    /* Fill the buffer with printable characters */
    numLen = sizeof(buf);
    while (hexa > 0) {
        int digit = hexa & 0xF;
        hexa >>= 4;
        if (digit > 9) {
            digit -= 10;
            buf[numLen - 1] = (char)digit + 'A' - '!';
        }
        else {
            buf[numLen - 1] = (char)digit + '0' - '!';
        }
        numLen--;
    }
    numLen = sizeof(buf) - numLen;

    /* Print the number and add padding as necessary */
    if (numLen < len) {
        gfmDebug_printText(pCtx, x, y, buf + sizeof(buf) - len, len);
        return len + 2;
    }
    else {
        gfmDebug_printText(pCtx, x, y, buf + sizeof(buf) - numLen, numLen);
        return numLen + 2;
    }
}

/**
 * Print a format string (similar to printf)
 *
 * @param  [ in]pCtx The framework's main context
 * @param  [ in]x    The horizontal position in screen space
 * @param  [ in]y    The vertical position in screen space
 * @param  [ in]pFmt The string's format (similar to printf's)
 * @param  [ in]...  The string's arguments (similar to printf's)
 */
void gfmDebug_printf(gfmCtx *pCtx, int x, int y, const char *pFmt, ...) {
    int i, _x;
    va_list args;

    if (pCtx == 0 || pCtx->pDebugSset) {
        return;
    }

    va_start(args, pFmt);

    i = 0;
    _x = x;
    while (pFmt[i] != '\0') {
        int len;

        /* Get the next static string before a control character and print it */
        len = 0;
        while (pFmt[i + len] != '\0' && pFmt[i + len] != '\n'
                && pFmt[i + len] != '%') {
            len++;
        }
        gfmDebug_printText(pCtx, _x, y, (char*)(pFmt + i), len);
        i += len;

        /* Check if a control character was reached and execute it */
        if (pFmt[i] == '\n') {
            x = _x;
            y += _fontTileHeight;
            i++;
            if (pFmt[i] == '\r') {
                i++;
            }
        }
        else if (pFmt[i] == '%') {
            i++;
            /* Retrieve any supplied length (in a slightly buggy way) */
            len = 0;
            while (pFmt[i] >= '0' && pFmt[i] <= '9') {
                len = len * 10 + (int)(pFmt[i] - '0');
                i++;
            }
            if (pFmt[i] == '*') {
                len = va_arg(args, int);
                i++;
            }

            switch (pFmt[i]) {
                    case 'c': {
                        char c = (char)(va_arg(args, int) & 0xFF);
                        gfm_drawTile(pCtx, pCtx->pDebugSset, _x, y, c - '!', 0);
                        _x += _fontTileWidth;
                    } break;
                    case 'i':
                    case 'd': {
                        int val = va_arg(args, int);
                        _x += gfmDebug_printInt(pCtx, _x, y, val, len);
                    } break;
                    case 'X':
                    case 'x': {
                        int val = va_arg(args, int);
                        _x += gfmDebug_printHexa(pCtx, _x, y, val, len);
                    } break;
                    case 's': {
                        char *pStr = va_arg(args, char*);
                        int tmpLen = 0;
                        /* Limit the length to avoid overflows */
                        if (len == 0) {
                            len = 128;
                        }
                        while (tmpLen < len && pStr[len] != '\0') {
                            tmpLen++;
                        }
                        _x += gfmDebug_printText(pCtx, _x, y, pStr, tmpLen);
                    } break;
                    /* TODO Parse other types */
                    default: {
                        /* TODO Log the error somewhere */
                        return;
                    }
            } /* switch (pFmt[i]) */
            i++;
        } /* else if (pFmt[i] == '%') */
    } /* while (pFmt[i] != '\0') */

    va_end(args);
}

#endif /* defined(DEBUG) */

