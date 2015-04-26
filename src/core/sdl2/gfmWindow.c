/**
 * @file src/core/gfmWindow.c
 * 
 * Handle operations on the window
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmWindow_bkend.h>

#include <SDL2/SDL_video.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct stGFMWindow {
    /** Actual window (managed by SDL2) */
    SDL_Window *pSDLWindow;
    /** Device's width */
    int devWidth;
    /** Device's height */
    int devHeight;
    /** Window's width */
    int width;
    /** Window's height */
    int height;
    /** Whether we are currently in full-screen mode */
    int isFullScreen;
    /** Current resolution; -1 if not set or custom */
    int curResolution;
    /** How many resolutions are supported by this device */
    int resCount;
    /** List of possible width resolutions */
    int *pWidths;
    /** List of possible height resolutions */
    int *pHeights;
    /** List of possible refresh rates */
    int *pRefRates;
};

/** 'Exportable' size of gfmWindow */
const int sizeofGFMWindow = sizeof(struct stGFMWindow);

/**
 * Alloc a new gfmWindow structure
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfmWindow_getNew(gfmWindow **ppCtx) {
    gfmRV rv;
    
    // Sanithe the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the window structure
    *ppCtx = (gfmWindow*)malloc(sizeofGFMWindow);
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Initialize every of the window's fields
    memset(*ppCtx, 0x00, sizeof(gfmWindow));
    (*ppCtx)->curResolution = -1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated gfmWindow
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmWindow_free(gfmWindow **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Make sure all memory is released
    gfmWindow_clean(*ppCtx);
    // Dealloc the window
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Whether the window was initialized
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmWindow_wasInit(gfmWindow *pCtx) {
    gfmRV rv;
    
    // Check whether the window was initialized or not
    if (!pCtx)
        rv = GFMRV_ARGUMENTS_BAD;
    else if (pCtx->pSDLWindow)
        rv = GFMRV_TRUE;
    else
        rv = GFMRV_FALSE;
    
    return rv;
}

/**
 * Create a list with all possible window resolutions and refresh rate; This
 * will depend on the actual backend, but the refresh rate may only be
 * meaningful when on full-screen
 * 
 * @param  pCount How many resolutions were found
 * @param  pCtx   Window context (will store the resolutions list)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_ALLOC_FAILED
 */
gfmRV gfmWindow_queryResolutions(int *pCount, gfmWindow *pCtx) {
    gfmRV rv;
    int displayIndex, i, iret, numVideoDisplays;
    SDL_DisplayMode sdlMode;
    
    // Sanitize the arguments
    ASSERT(pCount, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Simply return pCount if it's already queried
    CASSERT(pCtx->resCount == 0, GFMRV_OK, __set_count);
    // TODO If resolutions were queries, update list?
    
    // Get how many displays there are
    numVideoDisplays = SDL_GetNumVideoDisplays();
    ASSERT(numVideoDisplays > 0, GFMRV_INTERNAL_ERROR);
    
    // TODO support multiple displays
    displayIndex = 0;
    
    // Get how many different display modes there are
    pCtx->resCount = SDL_GetNumDisplayModes(displayIndex);
    ASSERT(pCtx->resCount > 0, GFMRV_INTERNAL_ERROR);
    
    // Get the device's default resolution
    iret = SDL_GetDisplayMode(displayIndex, 0, &sdlMode);
    ASSERT(iret == 0, GFMRV_INTERNAL_ERROR);
    pCtx->devWidth = sdlMode.w;
    pCtx->devHeight = sdlMode.h;
    
    // Allocate the buffers required to store the resolutions
    pCtx->pWidths = (int*)malloc(sizeof(int)*pCtx->resCount);
    ASSERT(pCtx->pWidths, GFMRV_ALLOC_FAILED);
    
    pCtx->pHeights = (int*)malloc(sizeof(int)*pCtx->resCount);
    ASSERT(pCtx->pHeights, GFMRV_ALLOC_FAILED);
    
    pCtx->pRefRates = (int*)malloc(sizeof(int)*pCtx->resCount);
    ASSERT(pCtx->pRefRates, GFMRV_ALLOC_FAILED);
    
    // Get every possible resolution
    i = 0;
    while (i < pCtx->resCount) {
        iret = SDL_GetDisplayMode(displayIndex, i, &sdlMode);
        ASSERT(iret == 0, GFMRV_INTERNAL_ERROR);
        
        // TODO check if it has a valid color mode(?)
        pCtx->pWidths[i] = sdlMode.w;
        pCtx->pHeights[i] = sdlMode.h;
        pCtx->pRefRates[i] = sdlMode.refresh_rate;
        i++;
    }
    
    rv = GFMRV_OK;
__set_count:
    *pCount = pCtx->resCount;
__ret:
    // Clean up memory, on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        if (pCtx->pWidths)
            free(pCtx->pWidths);
        pCtx->pWidths = 0;
        if (pCtx->pHeights)
            free(pCtx->pHeights);
        pCtx->pHeights = 0;
        if (pCtx->pRefRates)
            free(pCtx->pRefRates);
        pCtx->pRefRates = 0;
        pCtx->devWidth = 0;
        pCtx->devHeight = 0;
    }
    return rv;
}

/**
 * Get a resolution; if gfmWindow_queryResolutions wasn't previously called, it
 * will be automatically called
 * 
 * @param  pWidth   A possible window's width
 * @param  pHeight  A possible window's height
 * @param  pRefRate A possible window's refresh rate
 * @param  pCtx     The window context
 * @param  index    Resolution to be read (0 is the default resolution)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                  GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmWindow *pCtx, int index) {
    gfmRV rv;
    int count;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pRefRate, GFMRV_ARGUMENTS_BAD);
    
    // Check if the resolutions were already queried (do so, if necessary)
    rv = gfmWindow_queryResolutions(&count, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Check if its a valid index
    ASSERT(index > 0 && index < count, GFMRV_INVALID_INDEX);
    
    // Get the resolution
    *pWidth = pCtx->pWidths[index];
    *pHeight = pCtx->pHeights[index];
    *pRefRate = pCtx->pRefRates[index];
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize (i.e., create) a window with the desired dimensions; If the
 * resolution is greater than the device's, then the device's resolution shall
 * be used
 * 
 * @param  pCtx            The window context
 * @param  width           The desired width
 * @param  height          The desired height
 * @param  pName           The game's title, in a NULL terminated string
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                         GFMRV_INTERNAL_ERROR, GFMRV_INVALID_WIDTH,
 *                         GFMRV_INVALID_HEIGHT
 */
gfmRV gfmWindow_init(gfmWindow *pCtx, int width, int height, char *pName,
        int isUserResizable) {
    gfmRV rv;
    int count;
    SDL_WindowFlags flags;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pName, GFMRV_ARGUMENTS_BAD);
    
    // Check that both width and height are valid
    rv = gfmWindow_queryResolutions(&count, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    ASSERT(width <= pCtx->devWidth, GFMRV_INVALID_WIDTH);
    ASSERT(height <= pCtx->devHeight, GFMRV_INVALID_HEIGHT);
    
    // Set wheter resizing of the window is enabled
    if (isUserResizable)
        flags = SDL_WINDOW_RESIZABLE;
    else
        flags = 0;
    
    // Create a window
    pCtx->pSDLWindow = SDL_CreateWindow(pName, SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    ASSERT(pCtx->pSDLWindow, GFMRV_INTERNAL_ERROR);
    
    // Set the current dimensions
    pCtx->width = width;
    pCtx->height = height;
    pCtx->isFullScreen = 0;
    pCtx->curResolution = -1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize (i.e., create) a full screen window with the desired resolution
 * 
 * @param  pCtx            The window context
 * @param  index           Resolution to be used (0 is the default resolution)
 * @param  pName           The game's title, in a NULL terminated string
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                         GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_initFullScreen(gfmWindow *pCtx, int resIndex, char *pName, 
        int isUserResizable);

/**
 * Clean up (i.e., close) the window
 * 
 * @param  pCtx The window context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmWindow_clean(gfmWindow *pCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the window
    if (pCtx->pSDLWindow) {
        // TODO revert screen to it's original mode? (is it required?)
        SDL_DestroyWindow(pCtx->pSDLWindow);
    }
    pCtx->pSDLWindow = 0;
    // Clean up every array
    if (pCtx->pWidths)
        free(pCtx->pWidths);
    pCtx->pWidths = 0;
    if (pCtx->pHeights)
        free(pCtx->pHeights);
    pCtx->pHeights = 0;
    if (pCtx->pRefRates)
        free(pCtx->pRefRates);
    pCtx->pRefRates = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Resize the window to the desired dimensions;
 * This function won't work if the screen is in fullscreen mode! In that case,
 * use gfmWindow_setResolution
 * 
 * @param  pCtx   The window context
 * @param  width  The desired width
 * @param  height The desired height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_IS_FULLSCREEN
 */
gfmRV gfmWindow_setDimensions(gfmWindow *pCtx, int width, int height) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check that the window isn't in fullscreen mode
    ASSERT(!pCtx->isFullScreen, GFMRV_WINDOW_IS_FULLSCREEN);
    
    // Check that the dimensions are valid
    ASSERT(width <= pCtx->devWidth, GFMRV_INVALID_WIDTH);
    ASSERT(height <= pCtx->devHeight, GFMRV_INVALID_HEIGHT);
    
    // Resize the window
    SDL_SetWindowSize(pCtx->pSDLWindow, width, height);
    pCtx->width = width;
    pCtx->height = height;
    pCtx->curResolution = -1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Make the game go full-screen
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setFullScreen(gfmWindow *pCtx) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check that the window isn't in fullscreen mode
    ASSERT(!pCtx->isFullScreen, GFMRV_WINDOW_MODE_UNCHANGED);
    
    // Try to make it fullscrren
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullScreen = 1;
    
    // Update the resolution
    if (pCtx->curResolution != -1) {
        rv = gfmWindow_setResolution(pCtx, pCtx->curResolution);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Make the game go windowed;
 * The window's dimensions will be kept!
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setWindowed(gfmWindow *pCtx) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check that the window isn't in fullscreen mode
    ASSERT(pCtx->isFullScreen, GFMRV_WINDOW_MODE_UNCHANGED);
    
    // Try to make it windowed
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, 0);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullScreen = 0;
    
    // Recover the previous width
    rv = gfmWindow_setDimensions(pCtx, pCtx->width, pCtx->height);
    // On failure, try to set it to any other resolution
    if (rv != GFMRV_OK) {
        int i;
        
        i = 0;
        while (i < pCtx->resCount) {
            rv = gfmWindow_setDimensions(pCtx, pCtx->pWidths[i],
                    pCtx->pHeights[i]);
            if (rv == GFMRV_OK)
                break;
            i++;
        }
        ASSERT(i < pCtx->resCount, GFMRV_INTERNAL_ERROR);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the window's resolution;
 * If the window is in full screen mode, it's resolution and refresh rate will
 * be modified; Otherwise, only it's dimension's will be modified
 * 
 * @param  pCtx  The window context
 * @param  index Resolution to be used (0 is the default resolution)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_setResolution(gfmWindow *pCtx, int resIndex) {
    gfmRV rv;
    int displayIndex, irv;
    SDL_DisplayMode sdlMode;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(resIndex >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the index is valid
    ASSERT(resIndex < pCtx->resCount, GFMRV_INVALID_INDEX);
    
    // TODO enable multiple displays
    displayIndex = 0;
    
    // Get the window's mode (since the color info would be missing)
    irv = SDL_GetDisplayMode(displayIndex, resIndex, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    // Modify the resolution
    if (pCtx->isFullScreen) {
        irv = SDL_SetWindowDisplayMode(pCtx->pSDLWindow, &sdlMode);
        ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    }
    else {
        rv = gfmWindow_setDimensions(pCtx, pCtx->pWidths[resIndex], pCtx->pHeights[resIndex]);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    pCtx->curResolution = resIndex;
    rv = GFMRV_OK;
__ret:
    return rv;
}

