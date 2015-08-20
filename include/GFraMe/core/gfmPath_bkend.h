/**
 * @file include/GFraMe/core/gfmPath_bkend.h
 * 
 * Module for retrieving default paths
 */
#ifndef __GFMPATH_BKEND_H_
#define __GFMPATH_BKEND_H_

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gframe.h>

/**
 * Get the directory to this application's local files. It's used, by default,
 * as the preferred directory to saving logs and save files.
 * 
 * NOTE: This will only be valid if the game title was set!
 * NOTE2: This functions necessarily stores the path in a newly alloc'ed string
 * 
 * The following directories are used:
 *   Android: /data/data/concat(organization, title)/
 *   Windows: %APPDATA%\concat(organization, title)\
 *    Debian: ~/.local/shared/concat(organization, title)/
 * 
 * @param  ppStr The retrieved path
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ALLOC_FAILED, GFMRV_ARGUMENTS_BAD,
 *               GFMRV_TITLE_NOT_SET, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmPath_getLocalPath(gfmString **ppStr, gfmCtx *pCtx);

/**
 * Get the path to the directory from which the game was exectued
 * 
 * @param  ppStr The retireved path
 * @return       GFMRV_OK, GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmPath_getRunningPath(gfmString **ppStr);

#endif

