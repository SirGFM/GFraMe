/**
 * @file src/core/sdl2/gfmBackend.c
 * 
 * Initializes and finalize a backend
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmBackend_bkend.h>

#include <SDL2/SDL.h>

/** Whether SDL2 was already initialized */
static int isSDLInit = 0;

/**
 * Initialize a backend
 * 
 * @return GFMRV_OK, GFMRV_INTERNAL_ERROR, GFMRV_BACKEND_ALREADY_INITIALIZED
 */
gfmRV gfmBackend_init() {
    gfmRV rv;
    int irv;
    
    ASSERT(isSDLInit == 0, GFMRV_BACKEND_ALREADY_INITIALIZED);
    irv = SDL_Init(0);
    ASSERT(irv >= 0, GFMRV_INTERNAL_ERROR);
    
    isSDLInit = 1;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Finalize a backend
 * 
 * @return GFMRV_OK, GFMRV_BACKEND_NOT_INITIALIZED
 */
gfmRV gfmBackend_finalize() {
    gfmRV rv;
    
    ASSERT(isSDLInit == 1, GFMRV_BACKEND_NOT_INITIALIZED);
    SDL_Quit();
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

