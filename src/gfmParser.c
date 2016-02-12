/**
 * @file src/gfmParser.c
 * 
 * Generic parser for files generated by the tiled plugin
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe_int/gfmParserCommon.h>

#include <stdlib.h>
#include <string.h>

struct stGFMParsedObject {
    /** Type of the object */
    gfmParserType type;
    /** A NULL-terminated string representing the in-game type */
    char *pType;
    /** Object's horizontal position (when applied) */
    int x;
    /** Object's vertical position (when applied) */
    int y;
    /** Object's width (when applied) */
    int width;
    /** Object's height (when applied) */
    int height;
    /** Array with all properties key, value pair */
    char **ppProperties;
    /** Number of properties parsed into this object */
    int propertiesLen;
};
typedef struct stGFMParsedObject gfmParsedObject;

/** The gfmParser struct */
struct stGFMParser {
    /** Currently opened file */
    gfmFile *pFile;
    /** Logger obtained when opening the file (on gfmParser_init) */
    gfmLog *pLog;
    /** The parsed object */
    gfmParsedObject object;
    /** Buffer used to read a string from the file (NULL-terminated) */
    char *pReadBuf;
    /** Total length of the 'pReadBuf' buffer */
    int readBufLen;
    /** Buffer for the object's type */
    char *pTypeBuf;
    /** Total length of the 'pTypeBuf' buffer */
    int typeBufLen;
    /** Buffer used to stored all properties (keys and values), so they can be
     * easily indexed/pointed to on ppProps */
    char *pPropsBuf;
    /** Total length of the 'pPropsBuf' buffer */
    int propsBufLen;
    /** Array for easily pointing to all of a objects properties */
    char **ppProps;
    /** Total length of the 'ppProps' buffer */
    int propsLen;
};

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Parse any number of properties into the current object
 *
 * @param  [ in]pCtx The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmParser_parseProperties(gfmParser *pCtx) {
    /** GFraMe return value */
    gfmRV rv;
    /** The number of attributes found */
    int len;
    /** Whether the file position was pushed */
    int didPush;

    /* Store the current position */
    didPush = 0;
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;

    pCtx->object.propertiesLen = 0;
    len = 0;
    while (1) {
        /** Flag that every pointer should be realloc'ed (in the argv-like
         * array) */
        int doRecache;
        /** Length of the latest key */
        int keyLen;
        /** Length of the latest value */
        int valLen;

        /* Clear the flag that every pointer should be rellocated */
        doRecache = 0;

        /* Check that there's another property */
        rv = gfmParser_parseStringStatic(pCtx->pFile, "[");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pCtx->pLog);
        if (rv == GFMRV_FALSE) {
            break;
        }
        /* Retrieve the key */
        rv = gfmParser_getString(&(pCtx->pReadBuf), &(pCtx->readBufLen),
                pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        rv = gfmParser_parseStringStatic(pCtx->pFile, ",");
        ASSERT_LOG(rv == GFMRV_TRUE, GFMRV_PARSER_BAD_TOKEN, pCtx->pLog);
        /* Store the key */
        keyLen = strlen(pCtx->pReadBuf);
        if (len + keyLen + 1 > pCtx->propsBufLen) {
            /* Expand the buffer as necessary */
            pCtx->pPropsBuf = (char*)realloc(pCtx->pPropsBuf, len + keyLen + 1);
            ASSERT_LOG(pCtx->pPropsBuf, GFMRV_ALLOC_FAILED, pCtx->pLog);
            pCtx->propsBufLen = len + keyLen + 1;
            /* Force all pointers to be rellocated */
            doRecache = 1;
        }
        memcpy(pCtx->pPropsBuf + len, pCtx->pReadBuf, keyLen + 1);

        /* Retrieve the value */
        rv = gfmParser_getString(&(pCtx->pReadBuf), &(pCtx->readBufLen),
                pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        rv = gfmParser_parseStringStatic(pCtx->pFile, "]");
        ASSERT_LOG(rv == GFMRV_TRUE, GFMRV_PARSER_BAD_TOKEN, pCtx->pLog);
        /* Store the key */
        valLen = strlen(pCtx->pReadBuf);
        if (len + keyLen + 1 + valLen + 1 > pCtx->propsBufLen) {
            /* Expand the buffer as necessary */
            pCtx->pPropsBuf = (char*)realloc(pCtx->pPropsBuf, len + keyLen + 1 +
                    valLen + 1);
            ASSERT_LOG(pCtx->pPropsBuf, GFMRV_ALLOC_FAILED, pCtx->pLog);
            pCtx->propsBufLen = len + keyLen + 1 + valLen + 1;
            /* Force all pointers to be rellocated */
            doRecache = 1;
        }
        memcpy(pCtx->pPropsBuf + len + keyLen + 1, pCtx->pReadBuf, valLen + 1);

        /* Append them to the properties array */
        if (pCtx->propsLen < (pCtx->object.propertiesLen + 1) * 2) {
            /* Expand it as necessary */
            pCtx->ppProps = (char**)realloc(pCtx->ppProps, sizeof(char**) *
                    (pCtx->object.propertiesLen + 1) * 2);
            ASSERT_LOG(pCtx->ppProps, GFMRV_ALLOC_FAILED, pCtx->pLog);

            pCtx->propsLen = (pCtx->object.propertiesLen + 1) * 2;
        }

        /* Recache only if there was something already on the buffer (note that
         * propertiesLen is the number of key,value pairs, and not the number of
         * elements) */
        if (doRecache && pCtx->object.propertiesLen > 0) {
            /** Iterate through all properties */
            int i;
            /** Offset to the next string */
            unsigned int nextDist;

            /* Get the distance from the first string to the second */
            nextDist = pCtx->ppProps[1] - pCtx->ppProps[0];

            /* Store the first string */
            pCtx->ppProps[0] = pCtx->pPropsBuf;
            i = 1;
            while (i < (pCtx->object.propertiesLen * 2) - 1) {
                unsigned int tmp;

                /* Get the distance from the current string to the next */
                tmp = pCtx->ppProps[i + 1] - pCtx->ppProps[i];
                /* Update the current string position, based on the previous */
                pCtx->ppProps[i] = pCtx->ppProps[i-1] + nextDist;
                /* Store the distance for the next iteration */
                nextDist = tmp;

                i++;
            }
            /* Update the current string position, based on the previous */
            pCtx->ppProps[i] = pCtx->ppProps[i-1] + nextDist;
        }
        /* Store the new property */
        pCtx->ppProps[pCtx->object.propertiesLen * 2] = pCtx->pPropsBuf + len;
        pCtx->ppProps[pCtx->object.propertiesLen * 2 + 1] = pCtx->pPropsBuf +
            len + keyLen + 1;

        pCtx->object.propertiesLen++;
        len += keyLen + valLen + 2;
    }

    /* Store the object's 'argv' */
    if (pCtx->object.propertiesLen > 0) {
        pCtx->object.ppProperties = pCtx->ppProps;
    }
    else {
        pCtx->object.ppProperties = 0;
    }

    /* Clear the previous push */
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;

    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        /* On failure, go back to the previous position */
        gfmFile_popPos(pCtx->pFile);
    }

    return rv;
}

/**
 * Retrieve the in-game type from the file and put it into pTypeBuf
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_INTERNAL_ERROR, ...
 */
static gfmRV gfmParser_parseStrType(gfmParser *pCtx) {
    char *pDest;
    gfmRV rv;
    int didPush, len;
    
    // Set default values
    didPush = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(gfmFile_isOpen(pCtx->pFile) == GFMRV_TRUE,
            GFMRV_PARSER_NOT_INITIALIZED, pCtx->pLog);
    
    // Store the current position
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;
    
    // Retrieve a string from the file
    rv = gfmParser_getString(&(pCtx->pReadBuf), &(pCtx->readBufLen),
            pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Make sure there in enough space to copy it into the type buffer
    len = strlen(pCtx->pReadBuf);
    if (pCtx->typeBufLen < len + 1) {
        pCtx->pTypeBuf = (char*)realloc(pCtx->pTypeBuf, len + 1);
        ASSERT_LOG(pCtx->pTypeBuf , GFMRV_ALLOC_FAILED, pCtx->pLog);
        pCtx->typeBufLen = len + 1;
    }
    // Copy it into that buffer
    pDest = memcpy(pCtx->pTypeBuf, pCtx->pReadBuf, len + 1);
    ASSERT_LOG(pDest == pCtx->pTypeBuf, GFMRV_INTERNAL_ERROR, pCtx->pLog);
    
    // Clear the previous push
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;
    
    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        // On failure, go back to the previous position
        rv = gfmFile_popPos(pCtx->pFile);
    }
    
    return rv;
}

/**
 * Parses an area; It expects a type string and 4 integers: x, y, width and
 * height
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmParser_parseArea(gfmParser *pCtx) {
    gfmRV rv;
    int didPush;
    
    // Set default values
    didPush = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(gfmFile_isOpen(pCtx->pFile) == GFMRV_TRUE,
            GFMRV_PARSER_NOT_INITIALIZED, pCtx->pLog);
    
    // Store the current position
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;
    
    // Set the object as an area
    pCtx->object.type = gfmParserType_area;
    // Parse its in-game type
    rv = gfmParser_parseStrType(pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    pCtx->object.pType = pCtx->pTypeBuf;
    // Parse its position
    rv = gfmParser_parseInt(&(pCtx->object.x), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmParser_parseInt(&(pCtx->object.y), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Parse its dimensions
    rv = gfmParser_parseInt(&(pCtx->object.width), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmParser_parseInt(&(pCtx->object.height), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Clear the previous push
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;
    
    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        // On failure, go back to the previous position
        rv = gfmFile_popPos(pCtx->pFile);
    }
    
    return rv;
}

/**
 * Parses an area; It expects a type string, 4 integers (x, y, width and
 * height) and any number of properties, each a pair of key and value, enclosed
 * by square brackets
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmParser_parseObject(gfmParser *pCtx) {
    gfmRV rv;
    int didPush;
    
    // Set default values
    didPush = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(gfmFile_isOpen(pCtx->pFile) == GFMRV_TRUE,
            GFMRV_PARSER_NOT_INITIALIZED, pCtx->pLog);
    
    // Store the current position
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;
    
    // Set the object as an area
    pCtx->object.type = gfmParserType_object;
    // Parse its in-game type
    rv = gfmParser_parseStrType(pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    pCtx->object.pType = pCtx->pTypeBuf;
    // Parse its position
    rv = gfmParser_parseInt(&(pCtx->object.x), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmParser_parseInt(&(pCtx->object.y), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Parse its dimensions
    rv = gfmParser_parseInt(&(pCtx->object.width), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmParser_parseInt(&(pCtx->object.height), pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Parse all of its properties
    rv = gfmParser_parseProperties(pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Clear the previous push
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;
    
    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        // On failure, go back to the previous position
        gfmFile_popPos(pCtx->pFile);
    }
    
    return rv;
}

/**
 * Parses attributes; It expects any number of properties, each a pair of key
 * and value, enclosed by square brackets
 *
 * @param  [ in]pCtx The parser context
 * @return           GFraMe return value
 */
static gfmRV gfmParser_parseAttributes(gfmParser *pCtx) {
    gfmRV rv;
    int didPush;
    
    /* Set default values */
    didPush = 0;
    
    /* Store the current position */
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;
    
    /* Set the object as attributes */
    pCtx->object.type = gfmParserType_attributes;
    
    /* Parse all of its properties */
    rv = gfmParser_parseProperties(pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    /* Check that at least one property was found */
    ASSERT_LOG(pCtx->object.propertiesLen > 0, GFMRV_PARSER_BAD_TOKEN,
            pCtx->pLog);
    
    /* Clear the previous push */
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;
    
    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        /* On failure, go back to the previous position */
        gfmFile_popPos(pCtx->pFile);
    }
    
    return rv;
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Alloc a new parser
 * 
 * @param  ppCtx The parser
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmParser_getNew(gfmParser **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the struct
    *ppCtx = (gfmParser*)malloc(sizeof(gfmParser));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmParser));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Close the file, clean any resources and free the parser
 * 
 * @param  ppCtx The parser
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmParser_free(gfmParser **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Make sure all resources are deallocated
    gfmParser_clean(*ppCtx);
    // Dealloc the struct
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a parser and open its file
 * 
 * @param  pParser     The parser
 * @param  pCtx        The game's context
 * @param  pFilename   File to be parsed (must be on assets folder)
 * @param  filenameLen Length of the file's path
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_PARSER_ALREADY_INITIALIZED, ...
 */
gfmRV gfmParser_init(gfmParser *pParser, gfmCtx *pCtx, char *pFilename,
        int filenameLen) {
    gfmRV isFileOpen;
    gfmLog *pLog;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Retrieve the logger
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    // Continue to sanitize arguments
    ASSERT_LOG(pParser, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pFilename, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(filenameLen > 0, GFMRV_ARGUMENTS_BAD, pLog);
    // Check that there's no previous file open
    isFileOpen = gfmFile_isOpen(pParser->pFile);
    ASSERT_LOG(pParser->pFile == 0 || isFileOpen == GFMRV_FALSE,
            GFMRV_PARSER_ALREADY_INITIALIZED, pLog);
    
    // Clean the parsed object
    memset(&(pParser->object), 0x0, sizeof(gfmParsedObject));
    // Open the file
    if (!(pParser->pFile)) {
        rv = gfmFile_getNew(&(pParser->pFile));
        ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    }
    rv = gfmFile_openAsset(pParser->pFile, pCtx, pFilename, filenameLen, 0/*isText*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    
    // Store the logger
    pParser->pLog = pLog;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Close the file but keep allocated resources, so the parser may be reused
 * 
 * @param  pCtx The parser
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmParser_reset(gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // It will return true only if there's a file and its open, so that's safe
    if (gfmFile_isOpen(pCtx->pFile) == GFMRV_TRUE) {
        rv = gfmFile_close(pCtx->pFile);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean all allocated resources and close any open file
 * 
 * @param  pCtx The parser
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmParser_clean(gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    gfmFile_free(&(pCtx->pFile));
    if (pCtx->pReadBuf) {
        free(pCtx->pReadBuf);
    }
    if (pCtx->pTypeBuf) {
        free(pCtx->pTypeBuf);
    }
    if (pCtx->pPropsBuf) {
        free(pCtx->pPropsBuf);
    }
    if (pCtx->ppProps) {
        free(pCtx->ppProps);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Parse the next object
 * 
 * @param  pCtx The parser
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *              GFMRV_PARSER_FINISHED, GFMRV_ALLOC_FAILED,
 *              GFMRV_PARSER_BAD_TOKEN
 */
gfmRV gfmParser_parseNext(gfmParser *pCtx) {
    gfmRV rv;
    int didParse, didPush;
    
    // Set default values
    didParse = 0;
    didPush = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(gfmFile_isOpen(pCtx->pFile) == GFMRV_TRUE,
            GFMRV_PARSER_NOT_INITIALIZED, pCtx->pLog);
    
    // Make sure we are at the next token and there's something to parse
    rv = gfmParser_ignoreBlank(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmFile_didFinish(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pCtx->pLog);
    if (rv == GFMRV_TRUE) {
        rv = GFMRV_PARSER_FINISHED;
        goto __ret;
    }
    
    // Store the current position
    rv = gfmFile_pushPos(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 1;
    
    // Try to parse an "object"
    if (didParse == 0) {
        rv = gfmParser_parseStringStatic(pCtx->pFile, "area");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pCtx->pLog);
        if (rv == GFMRV_TRUE) {
            // Parse the area
            rv = gfmParser_parseArea(pCtx);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
            didParse = 1;
        }
    }
    if (didParse == 0) {
        rv = gfmParser_parseStringStatic(pCtx->pFile, "obj");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pCtx->pLog);
        if (rv == GFMRV_TRUE) {
            // Parse the object
            rv = gfmParser_parseObject(pCtx);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
            didParse = 1;
        }
    }
    if (didParse == 0) {
        rv = gfmParser_parseStringStatic(pCtx->pFile, "attr");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pCtx->pLog);
        if (rv == GFMRV_TRUE) {
            // Parse the custom object
            rv = gfmParser_parseAttributes(pCtx);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        }
    }
    // Make sure something was parsed
    ASSERT_LOG(didParse, GFMRV_PARSER_BAD_TOKEN, pCtx->pLog);
    
    // Clear the previous push
    rv = gfmFile_clearLastPosStack(pCtx->pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    didPush = 0;
    
    rv = GFMRV_OK;
__ret:
    if (didPush != 0) {
        // On failure, go back to the previous position
        rv = gfmFile_popPos(pCtx->pFile);
    }
    
    return rv;
}

/**
 * Retrieve the parsed object's type
 * 
 * @param  pType The object's type
 * @param  pCtx  The parser
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *               GFMRV_PARSER_NO_OBJECT
 */
gfmRV gfmParser_getType(gfmParserType *pType, gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pType, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Get the attribute
    *pType = pCtx->object.type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the parsed object's position
 * 
 * @param  pX   The object's horizontal position
 * @param  pY   The object's vertical position
 * @param  pCtx The parser
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *              GFMRV_PARSER_NO_OBJECT, GFMRV_PARSER_INVALID_FIELD
 */
gfmRV gfmParser_getPos(int *pX, int *pY, gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pX, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pY, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Check if the type has this attribute
    ASSERT_LOG(pCtx->object.type == gfmParserType_area ||
            pCtx->object.type == gfmParserType_object,
            GFMRV_PARSER_INVALID_OBJECT, pCtx->pLog);
    // Get the attribute
    *pX = pCtx->object.x;
    *pY = pCtx->object.y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the parsed object's dimensions
 * 
 * @param  pWidth  The object's width
 * @param  pHeight The object's height
 * @param  pCtx    The parser
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *                 GFMRV_PARSER_NO_OBJECT, GFMRV_PARSER_INVALID_FIELD
 */
gfmRV gfmParser_getDimensions(int *pWidth, int *pHeight, gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pWidth, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pHeight, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Check if the type has this attribute
    ASSERT_LOG(pCtx->object.type == gfmParserType_area ||
            pCtx->object.type == gfmParserType_object,
            GFMRV_PARSER_INVALID_OBJECT, pCtx->pLog);
    // Get the attribute
    *pWidth = pCtx->object.width;
    *pHeight = pCtx->object.height;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the string representing the object's in-game type (instead of the
 * 'parser type')
 * 
 * @param  pType The object's type
 * @param  pCtx  The paser
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *               GFMRV_PARSER_NO_OBJECT
 */
gfmRV gfmParser_getIngameType(char **ppType, gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppType, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Get the attribute
    *ppType = pCtx->object.pType;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the parsed object's 
 * 
 * @param  pNum How many properties the object has
 * @param  pCtx The parser
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *              GFMRV_PARSER_NO_OBJECT
 */
gfmRV gfmParser_getNumProperties(int *pNum, gfmParser *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pNum, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Check if the type has this attribute
    ASSERT_LOG(pCtx->object.type == gfmParserType_object ||
            pCtx->object.type == gfmParserType_attributes,
            GFMRV_PARSER_INVALID_OBJECT, pCtx->pLog);
    // Get the attribute
    *pNum = pCtx->object.propertiesLen;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the parsed object's 
 * 
 * @param  ppKey The property's key (a NULL-terminated string)
 * @param  ppVal The property's value (a NULL-terminated string)
 * @param  pCtx  The parser
 * @param  index The index of the property
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_PARSER_NOT_INITIALIZED,
 *               GFMRV_PARSER_NO_OBJECT, GFMRV_PARSER_INVALID_FIELD,
 *               GFMRV_INVALID_INDEX
 */
gfmRV gfmParser_getProperty(char **ppKey, char **pVal, gfmParser *pCtx,
        int index) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFile, GFMRV_PARSER_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppKey, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pVal, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that an object was parsed
    ASSERT_LOG(pCtx->object.type != gfmParserType_none, GFMRV_PARSER_NO_OBJECT,
            pCtx->pLog);
    
    // Check if the type has this attribute
    ASSERT_LOG(pCtx->object.type == gfmParserType_object ||
            pCtx->object.type == gfmParserType_attributes,
            GFMRV_PARSER_INVALID_OBJECT, pCtx->pLog);
    // Check if the index is valid
    ASSERT_LOG(index < pCtx->object.propertiesLen, GFMRV_ARGUMENTS_BAD,
            pCtx->pLog);
    // Get the attribute
    *ppKey = pCtx->object.ppProperties[index * 2];
    *pVal = pCtx->object.ppProperties[index * 2 + 1];
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

