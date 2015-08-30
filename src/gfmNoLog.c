/**
 * @file src/gfmLog.c
 * 
 * Module for logging stuff
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/core/gfmFile_bkend.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** The gfmLog struct */
struct stGFMLog {
    /** The current log file */
    gfmFile *pFile;
    /** The minimum level for logging */
    gfmLogLevel minLevel;
};

/** States for the log parser */
enum enGFMLogParser {
    gfmLogParser_waiting = 0,
    gfmLogParser_getType,
    gfmLogParser_max
};
typedef enum enGFMLogParser gfmLogParser;

/**
 * Alloc a new logger
 * 
 * @param  ppCtx The logger
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmLog_getNew(gfmLog **ppCtx) {
    return GFMRV_OK;
}

/**
 * Close the file and clean the logger
 * 
 * @param  ppCtx The logger
 * @reutrn       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmLog_free(gfmLog **ppCtx) {
    return GFMRV_OK;
}

/**
 * Initialize the logger
 * 
 * @param  pLog  The logger context
 * @param  pCtx  The game's context
 * @param  level The minimum logging level
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_LOG_ALREADY_INITIALIZED,
 *               GFMRV_COULDNT_OPEN_FILE, GFMRV_LOG_INVALID_LEVEL
 */
gfmRV gfmLog_init(gfmLog *pLog, gfmCtx *pCtx, gfmLogLevel level) {
    return GFMRV_OK;
}

/**
 * Close the log file and clean resources
 * 
 * @param  pLog  The logger context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmLog_clean(gfmLog *pCtx) {
    return GFMRV_OK;
}

/**
 * Write a string to the file
 * 
 * @param  pCtx The logger
 * @param  pStr The string
 * @param  len  The string's length
 */
static gfmRV gfmLog_logString(gfmLog *pCtx, char *pStr, int len) {
    return GFMRV_OK;
}

/**
 * Log an integer
 * 
 * @param  pCtx The logger
 * @param  val  Value to be logged
 */
static gfmRV gfmLog_logInt(gfmLog *pCtx, int val) {
    return GFMRV_OK;
}

/**
 * Log the current time to the file
 * 
 * @param  pCtx The logger
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_LOG_NOT_INITIALIZED,
 *              GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmLog_logTime(gfmLog *pCtx) {
    return GFMRV_OK;
}

