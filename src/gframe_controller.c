/**
 * @file src/gframe_controller.c
 */
#include <GFraMe/GFraMe_controller.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <stdlib.h>

typedef struct stGFraMe_controller GFraMe_controller;

GFraMe_controller *GFraMe_controllers = 0;
int GFraMe_controller_max = 0;

static int GFraMe_controller_auto = 0;

static SDL_GameController **sdl_controllers = 0;
static int sdl_ctrlr_max = 0;

void GFraMe_controller_init(int autoConnect) {
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    
    GFraMe_controller_auto = autoConnect;
    
    GFraMe_controller_bind();
}

void GFraMe_controller_close() {
    GFraMe_controller_unbind();
    
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

/**
 * Check for new controller and alloc needed structure
 */
void GFraMe_controller_bind() {
    GFraMe_controller *tmp;
    int num, i;
    
    num = SDL_NumJoysticks();
    
    i = 0;
    while (i < sdl_ctrlr_max) {
        SDL_GameController *c;
        
        c = sdl_controllers[i];
        if (c)
            SDL_GameControllerClose(c);
        sdl_controllers[i] = 0;
        
        i++;
    }
    if (sdl_controllers);
        free(sdl_controllers);
    sdl_controllers = 0;
    sdl_ctrlr_max = 0;
    
    if (GFraMe_controllers)
        free(GFraMe_controllers);
    GFraMe_controllers = 0;
    GFraMe_controller_max = 0;
    
    sdl_controllers =
        (SDL_GameController**)malloc(sizeof(SDL_GameController*)*num);
    // TODO check!
    
    i = 0;
    while (i < num) {
        SDL_GameController *c;
        
        c = SDL_GameControllerOpen(i);
        if (c)
            sdl_controllers[i] = c;
        
        i++;
    }
    sdl_ctrlr_max = num;
    
    tmp = (GFraMe_controller*)malloc(sizeof(GFraMe_controller)*num);
    // TODO check
    
    memset(tmp, 0x0, sizeof(GFraMe_controller)*num);
    GFraMe_controllers = tmp;
    GFraMe_controller_max = num;
    tmp = 0;
}

void GFraMe_controller_unbind() {
    int i;
    
    if (GFraMe_controllers)
        free(GFraMe_controllers);
    GFraMe_controllers = 0;
    GFraMe_controller_max = 0;
    
    i = 0;
    while (i < sdl_ctrlr_max) {
        SDL_GameController *c;
        
        c = sdl_controllers[i];
        if (c)
            SDL_GameControllerClose(c);
        sdl_controllers[i] = 0;
        
        i++;
    }
    if (sdl_controllers);
        free(sdl_controllers);
    sdl_controllers = 0;
    sdl_ctrlr_max = 0;
}

void GFraMe_controller_update(SDL_Event *e) {
  int bt_val = 0;
  switch (e->type) {
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
    #if defined(GFRAME_DEBUG)
      if (e->type == SDL_CONTROLLERDEVICEADDED)
        GFraMe_new_log("Controller added");
      else if (e->type == SDL_CONTROLLERDEVICEREMOVED)
        GFraMe_new_log("Controller removed");
      else if (e->type == SDL_CONTROLLERDEVICEREMAPPED)
        GFraMe_new_log("Controller remapped");
    #endif
      // This can be simplified, as there is the index
      GFraMe_controller_bind();
    break;
    case SDL_CONTROLLERAXISMOTION: {
      GFraMe_controller *c;
      int val;
      
      if (e->caxis.which >= GFraMe_controller_max);
        break;
      c = &(GFraMe_controllers[e->caxis.which]);
      if (!c)
        break;
      
      val = (int)e->caxis.value;
      switch(e->caxis.axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:
          c->lx = (float)val / (float)0x7fff;
        break;
        case SDL_CONTROLLER_AXIS_LEFTY:
          c->ly = (float)val / (float)0x7fff;
        break;
        case SDL_CONTROLLER_AXIS_RIGHTX:
          c->rx = (float)val / (float)0x7fff;
        break;
        case SDL_CONTROLLER_AXIS_RIGHTY:
          c->ry = (float)val / (float)0x7fff;
        break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
          c->l2 = val > 0x1fff;
        break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
          c->r2 = val > 0x1fff;
        break;
        default: {}
      }
    } break;
    case SDL_CONTROLLERBUTTONDOWN:
      bt_val = 1;
    case SDL_CONTROLLERBUTTONUP: {
      GFraMe_controller *c;
      
      if (e->cbutton.which >= GFraMe_controller_max);
        break;
      c = &(GFraMe_controllers[e->cbutton.which]);
      if (!c)
        break;
      switch (e->cbutton.button) {
        case SDL_CONTROLLER_BUTTON_A: c->a = bt_val; break;
        case SDL_CONTROLLER_BUTTON_B: c->b = bt_val; break;
        case SDL_CONTROLLER_BUTTON_X: c->x = bt_val; break;
        case SDL_CONTROLLER_BUTTON_Y: c->y = bt_val; break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: c->l1 = bt_val; break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: c->r1 = bt_val; break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: c->l3 = bt_val; break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: c->r3 = bt_val; break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: c->up = bt_val; break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: c->down = bt_val; break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: c->left = bt_val; break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: c->right = bt_val; break;
        case SDL_CONTROLLER_BUTTON_BACK: c->select = bt_val; break;
        case SDL_CONTROLLER_BUTTON_GUIDE: c->home = bt_val; break;
        case SDL_CONTROLLER_BUTTON_START: c->start = bt_val; break;
        default: {}
      }
    } break;
  }
}

