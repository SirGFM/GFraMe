/**
 * @file include/GFraMe/gfmLog.h
 * 
 * Module for logging stuff
 */
#ifndef __GFMLOG_STRUCT__
#define __GFMLOG_STRUCT__

/** 'Export' the gfmLog struct */
typedef struct stGFMLog gfmLog;

/** Possible levels for logging */
enum enGFMLogLevel {
    gfmLog_none = 0,
    gfmLog_debug,
    gfmLog_warn,
    gfmLog_info,
    gfmLog_error,
    gfmLog_max
};
typedef enum enGFMLogLevel gfmLogLevel;

#endif /* __GFMLOG_STRUCT__ */

#ifndef __GFMLOG_H__
#define __GFMLOG_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/**
 * Alloc a new logger
 * 
 * @param  ppCtx The logger
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmLog_getNew(gfmLog **ppCtx);

/**
 * Close the file and clean the logger
 * 
 * @param  ppCtx The logger
 * @reutrn       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmLog_free(gfmLog **ppCtx);

/**
 * Initialize the logger
 * 
 * @param  pLog  The logger context
 * @param  pCtx  The game's context
 * @param  level The minimum logging level
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_LOG_ALREADY_INITIALIZED,
 *               GFMRV_COULDNT_OPEN_FILE, GFMRV_LOG_INVALID_LEVEL
 */
gfmRV gfmLog_init(gfmLog *pLog, gfmCtx *pCtx, gfmLogLevel level);

/**
 * Close the log file and clean resources
 * 
 * @param  pLog  The logger context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmLog_clean(gfmLog *pCtx);

#  ifdef GFM_NO_LOG
/**
 * Ignores log implementations... >__<
 * (simply returns OK)
 */
#    define gfmLog_log(pCtx, level, pFmt, ...) GFMRV_OK
#  else /* GFM_NO_LOG */
/**
 * Log a message; The current time, file, function and line number will be
 * printed
 * 
 * @param  pCtx  The logger
 * @param  level This message's logging level
 * @param  pFmt  The message's format (similar to printf's)
 * @param  ...   The message's arguments (similar to printf's)
 */
#define gfmLog_log(pCtx, level, pFmt, ...) \
    gfmLog_simpleLog(pCtx, level, "[%s - %s (%d)] - "pFmt"\n", __FILE__, \
        __FUNCTION__, __LINE__, ##__VA_ARGS__)
        
/**
 * Log a message; The current time will be printed prior to the message
 * 
 * @param  pCtx  The logger
 * @param  level This message's logging level
 * @param  pFmt  The message's format (similar to printf's)
 * @param  ...   The message's arguments (similar to printf's)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, 
 */
gfmRV gfmLog_simpleLog(gfmLog *pCtx, gfmLogLevel level, char *pFmt, ...);

#  endif /* GFM_NO_LOG */

#endif /* __GFMLOG_H__ */

