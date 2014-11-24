/**
 * @file src/gframe_controller.c
 */
#include <SDL2/SDL_gamecontroller.h>

typedef struct stGFraMe_controller GFraMe_controller;

GFraMe_controller *GFraMe_controllers = 0;
int GFraMe_controller_max = 0;

static SDL_GameController *sdl_controllers;
static int sdl_ctrlr_max;

/**
 * Check for new controller and alloc needed structure
 */
GFraMe_ret GFraMe_controller_bind() {
    GFraMe_ret rv;
    GFraMe_controller *tmp;
    int num, i;
    
    num = SDL_NumJoysticks();
    // check
    GFraMe_assertRV(num != sdl_ctrlr_max, "No new controller",
        rv = GFraMe_ret_ok, __ret);
    
    i = 0;
    while (i < num) {
        
        i++;
    }
    // still needs to open controller
    
    tmp = (GFraMe_controller*)malloc(sizeof(GFraMe_controller)*num);
    // check
    
    memset(tmp, 0x0, sizeof(GFraMe_controller)*num);
    // Copy previous state
    if (controllers) {
        memcpy(tmp, GFraMe_controllers, max*sizeof(GFraMe_controller));
        
        free(GFraMe_controllers);
    }
    GFraMe_controllers = tmp;
    tmp = 0;
    
    rv = GFraMe_ret_ok;
_ret:
    if (rv != GFraMe_ret_ok && tmp)
        free(tmp);
    
    return rv;
}

void GFraMe_controller_unbind() {
    if (GFraMe_controllers)
        free(GFraMe_controllers);

    GFraMe_controllers = 0;
    max = 0;
}

