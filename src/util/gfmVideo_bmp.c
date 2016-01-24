/**
 * @file src/util/gfmVideo_bmp.c
 *
 * Loads a 24-bits bitmap file into a buffer
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe_int/gfmVideo_bmp.h>

#include <stdlib.h>

#define BMP_OFFSET_POS 0x0a
#define BMP_WIDTH_POS  0x12
#define BMP_HEIGHT_POS 0x16
#define READ_UINT(buffer) \
        ((0xff & buffer[0]) | ((buffer[1]<<8) & 0xff00) | \
        ((buffer[2]<<16) & 0xff0000) | ((buffer[3]<<24) & 0xff000000))

/**
 * Check if a given file is a bitmap
 *
 * @param  [ in]pFp  The opened file
 * @param  [ in]pLog The logger
 * @return           GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmVideo_isBmp(gfmFile *pFp, gfmLog *pLog) {
    gfmRV rv;
    int irv;
    char pBuffer[2];

    /* This isn't called externally, so no check is needed */

    /* Read the first two bytes and check that they are BM */
    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFp, 2/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    if (pBuffer[0] == 'B' && pBuffer[1] == 'M') {
        gfmLog_log(pLog, gfmLog_debug, "File is bitmap");
        rv = GFMRV_TRUE;
    }
    else {
        gfmLog_log(pLog, gfmLog_debug, "File isn't bitmap");
        rv = GFMRV_FALSE;
    }

__ret:
    return rv;
}

/**
 * Loads a 24-bits bitmap file into a 32-bits buffer
 *
 * NOTE: The buffer is callee alloc'ed, so it must be freed by the caller!
 *
 * @param  [out]pBuf     The loaded image
 * @param  [out]pWidth   The image's width
 * @param  [out]pHeight  The image's height
 * @param  [ in]pFp      The image's file
 * @param  [ in]pLog     The logger
 * @param  [ in]colorKey 24 bits, RGB Color to be treated as transparent
 * @return               GFMRV_OK, 
 */
gfmRV gfmVideo_loadFileAsBmp(char **pBuf, int *pWidth, int *pHeight,
        gfmFile *pFp, gfmLog *pLog, int colorKey) {
    char *pData, pBuffer[4];
    gfmRV rv;
    int bytesInRow, i, irv, rowOffset;
    volatile int height, dataOffset, width;

    /* Get the offset to the image's "data section" */
    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFp, BMP_OFFSET_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFp, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    dataOffset = READ_UINT(pBuffer);

    /* Get the image's dimensions */
    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFp, BMP_HEIGHT_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFp, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    height = READ_UINT(pBuffer);

    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFp, BMP_WIDTH_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFp, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    width = READ_UINT(pBuffer);

    rv = gfmLog_log(pLog, gfmLog_info, "Loading %ix%i image...", width, height);
    ASSERT(rv == GFMRV_OK, rv);

    /* Alloc the data array */
    pData = (char*)malloc(width * height * sizeof(char) * 4);
    ASSERT_LOG(pData, GFMRV_ALLOC_FAILED, pLog);

    /* Calculate how many bytes there are in a row of pixels */
    bytesInRow = width * 3;
    rowOffset = bytesInRow % 4;
    bytesInRow += rowOffset;

    /* Buffer the data (in the desired format) */
    rv = gfmFile_rewind(pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFp, dataOffset);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    /* Data is written starting by the last line */
    i = width * (height - 1);
    /* Start "alpha" with 0 (since the image is in RGB-24) */
    pBuffer[3] = 0;
    while (1) {
        /*int color, r, g, b, pos, n; */
        int r, g, b, pos, n;
        volatile int color;

        /* Read until the EOF */
        rv = gfmFile_readBytes(pBuffer, &n, pFp, 3/*numBytes*/);
        ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_FILE_EOF_REACHED, rv, pLog);
        if (rv == GFMRV_FILE_EOF_REACHED) {
            break;
        }

        /* Get the actual color */
        color = READ_UINT(pBuffer);
        /* Get each component */
        r = color & 0xff;
        g = (color >> 8) & 0xff;
        b = (color >> 16) & 0xff;

        /* Output the color */
        pos = i * 4;
        if (color == colorKey) {
            pData[pos + 0] = 0x00;
            pData[pos + 1] = 0x00;
            pData[pos + 2] = 0x00;
            pData[pos + 3] = 0x00;
        }
        else {
            pData[pos + 0] = (char)b & 0xff;
            pData[pos + 1] = (char)g & 0xff;
            pData[pos + 2] = (char)r & 0xff;
            pData[pos + 3] =   0xff;
        }

        /* Go to the next pixel in this row */
        i++;
        /* If a row was read, go back the current row and the "next" one */
        /* (actually the previous!) */
        if (i % width == 0) {
            i -= width * 2;
            /* Go to the next line on the file */
            if (rowOffset != 0) {
                rv = gfmFile_seek(pFp, rowOffset);
                ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            }
        }
    }

    *pBuf = pData;
    *pWidth = width;
    *pHeight = height;
    rv = GFMRV_OK;
__ret:
    /* Release the buffer, as it was already loaded into the texture */
    if (rv != GFMRV_OK && pData) {
        free(pData);
    }

    return rv;
}

