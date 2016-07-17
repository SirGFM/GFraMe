/**
 * @file src/core/sdl2/gfmPath.c
 * 
 * Module for retrieving default paths
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gfmUtils.h>
#include <GFraMe/core/gfmPath_bkend.h>
#include <GFraMe/gframe.h>

#if !defined(__APPLE__) && !defined(__MACH__)
#  include <SDL2/SDL_filesystem.h>
#endif
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_system.h>

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
gfmRV gfmPath_getLocalPath(gfmString **ppStr, gfmCtx *pCtx) {
    char *pPath;
#if !defined(__ANDROID__)
    char *pOrg, *pTitle;
#endif
    gfmRV rv;
    int doCopy, pathLen;
    
    // Initialize variable that may be cleaned
    pPath = 0;
    
    // Sanitize the arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppStr), GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Alloc a new string
    rv = gfmString_getNew(ppStr);
    ASSERT_NR(rv == GFMRV_OK);
    
#if defined(__ANDROID__) && __ANDROID__
    do {
        int mask;

        /* Check if there is a external storage (e.g., SD card) */
        mask = SDL_ANDROID_EXTERNAL_STORAGE_READ;
        mask = SDL_ANDROID_EXTERNAL_STORAGE_WRITE;
        if ((SDL_AndroidGetExternalStorageState() & mask) == mask) {
            /* Get the external path */
            pPath = (char*)SDL_AndroidGetExternalStoragePath();
        }
        else {
            /* Otherwise, retrieve the path to the internal one */
            pPath = (char*)SDL_AndroidGetInternalStoragePath();
        }
    } while (0);
#elif defined(__APPLE__) || defined(__MACH__)
    pPath = "./";
#else
    pOrg = 0;
    pTitle = 0;
    // Get the organization and title
    rv = gfm_getTitle(&pOrg, &pTitle, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the local path
    pPath = SDL_GetPrefPath(pOrg, pTitle);
#endif
    ASSERT(pPath, GFMRV_INTERNAL_ERROR);
    
    // Get the local path's len
    pathLen = 0;
    while (pPath[pathLen] != '\0')
        pathLen++;
    
    // Store the path
    doCopy = 1;
    rv = gfmString_init(*ppStr, pPath, pathLen, doCopy);
#if defined(__ANDROID__) && __ANDROID__
    // Clean the path, as it's a const pointer
    pPath = 0;
#endif
    // Check for errors
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
#if !defined(__APPLE__) && !defined(__MACH__)
    if (pPath)
        SDL_free(pPath);
#endif
    if (rv != GFMRV_ARGUMENTS_BAD && rv != GFMRV_OK)
        gfmString_free(ppStr);
    
    return rv;
}

/**
 * Get the path to the directory from which the game was exectued
 * 
 * @param  ppStr The retireved path
 * @return       GFMRV_OK, GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmPath_getRunningPath(gfmString **ppStr) {
    char *pTmpPath;
    gfmRV rv;
    int doCopy, tmpPathLen;
    
    // Initialize variables that are cleaned by the end
    pTmpPath = 0;
    
    // Sanitize the arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppStr), GFMRV_ARGUMENTS_BAD);
    
	// Get current directory
#if defined(__APPLE__) || defined(__MACH__)
	pTmpPath = "./";
#else
	pTmpPath = SDL_GetBasePath();
#endif
    ASSERT(pTmpPath, GFMRV_INTERNAL_ERROR);
    
    // Get the directory length
    tmpPathLen = 0;
    while (pTmpPath[tmpPathLen] != '\0')
        tmpPathLen++;
    
    // Alloc a new string
    rv = gfmString_getNew(ppStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Store the directory
    doCopy = 1;
    rv = gfmString_init(*ppStr, pTmpPath, tmpPathLen, doCopy);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    // Clean up memory
#if !defined(__APPLE__) && !defined(__MACH__)
    if (pTmpPath) {
	    SDL_free(pTmpPath);
        pTmpPath = 0;
    }
#endif
    if (rv != GFMRV_ARGUMENTS_BAD && rv != GFMRV_OK) {
        gfmString_free(ppStr);
    }
    
    return rv;
}

