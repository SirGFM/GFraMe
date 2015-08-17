/**
 * @file src/include/GFraMe_int/gfmParserCommon.h
 * 
 * Common functions used when parsing files
 */
#ifndef __GFMPARSERCOMMON_H_
#define __GFMPARSERCOMMON_H_

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>

/**
 * Check if a character if blank
 */
#define gfmParser_isBlank(c) \
        ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')

/**
 * Advance through all blank characters
 * 
 * @param  pFp The file to be read
 * @return     GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_READ_ERROR
 */
gfmRV gfmParser_ignoreBlank(gfmFile *pFp);

/**
 * Try to read a string from a file; On error, return the file to the previous
 * position
 * 
 * @param  ppStr   The parsed string (it may be pre-allocated, but its size must
 *                 be accurate(i.e., the buffer's size), in that case)
 * @param  pStrLen The string's length
 * @param  pFp     The current file
 * @return         GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_INTERNAL_ERROR, GFMRV_READ_ERROR,
 *                 GFMRV_TILEMAP_PARSING_ERROR, GFMRV_ALLOC_FAILED
 */
gfmRV gfmParser_getString(char **ppStr, int *pStrLen, gfmFile *pFp);

#define gfmParser_parseStringStatic(pFp, pStr) \
        gfmParser_parseString(pFp, pStr, sizeof(pStr) - 1)

/**
 * Try to match a string in file
 * 
 * @param  pFp    The current file
 * @param  pStr   The string to be matched
 * @param  strLen The string's length
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_INTERNAL_ERROR, GFMRV_READ_ERROR
 */
gfmRV gfmParser_parseString(gfmFile *pFp, char *pStr, int strLen);

/**
 * Try to read a int from a file
 * 
 * @param  pVal The parsed integer
 * @param  pFp  The current file
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *              GFMRV_READ_ERROR, GFMRV_TILEMAP_PARSING_ERROR
 */
gfmRV gfmParser_parseInt(int *pVal, gfmFile *pFp);

#endif /* __GFMPARSERCOMMON_H_ */

