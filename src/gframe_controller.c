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

static int GFraMe_controller_isInit = 0;
static int GFraMe_controller_auto = 0;

static SDL_GameController **sdl_controllers = 0;
static int sdl_ctrlr_max = 0;

void GFraMe_controller_init(int autoConnect) {
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    
    GFraMe_controller_auto = autoConnect;
    
    GFraMe_controller_bind();
    
    GFraMe_controller_isInit = 1;
}

void GFraMe_controller_close() {
    if (!GFraMe_controller_isInit)
        return;
    
    GFraMe_controller_unbind();
    
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    
    GFraMe_controller_isInit = 0;
}

/**
 * Check for new controller and alloc needed structure
 */
void GFraMe_controller_bind() {
    GFraMe_controller *tmp;
    int num, i;
    
    tmp = 0;
    num = SDL_NumJoysticks();
    
    GFraMe_controller_unbind();
    
    if (num > 0)
        sdl_controllers =
            (SDL_GameController**)malloc(sizeof(SDL_GameController*)*num);
    // TODO check!
    
    i = 0;
    while (i < num) {
        SDL_GameController *c;
        SDL_JoystickGUID guid;
        char *mapping;
        
        c = SDL_GameControllerOpen(i);
        if (c)
            sdl_controllers[i] = c;
        guid = SDL_JoystickGetDeviceGUID(i);
        mapping = SDL_GameControllerMappingForGUID(guid);
        
        i++;
    }
    sdl_ctrlr_max = num;
    
    if (num > 0) {
        tmp = (GFraMe_controller*)malloc(sizeof(GFraMe_controller)*num);
        // TODO check
        
        memset(tmp, 0x0, sizeof(GFraMe_controller)*num);
    }
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
    if (sdl_controllers)
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
      
      if (e->caxis.which >= GFraMe_controller_max)
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
      
      if (e->cbutton.which >= GFraMe_controller_max)
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

void GFraMe_controller_printStates() {
    int i;
    
    GFraMe_new_log("\n\n=========================================");
    GFraMe_new_log(" Controllers state");
    GFraMe_new_log("-----------------------------------------");
    GFraMe_new_log(" %i controller connecteds", GFraMe_controller_max);
    
    i = 0;
    while (i < GFraMe_controller_max) {
        GFraMe_controller *c;
        #define PRINT_PRESSED(att) \
            ((c->att)?"pressed":"not pressed")
        
        c = &(GFraMe_controllers[i]);
        
        GFraMe_new_log("-----------------------------------------");
        GFraMe_new_log("  Controller %02d:", i);
        GFraMe_new_log("    left axis: (%0.2f, %0.2f)", c->lx, c->ly);
        GFraMe_new_log("    right axis: (%0.2f, %0.2f)", c->rx, c->ry);
        GFraMe_new_log("       UP: %s", PRINT_PRESSED(up));
        GFraMe_new_log("     DOWN: %s", PRINT_PRESSED(down));
        GFraMe_new_log("     LEFT: %s", PRINT_PRESSED(left));
        GFraMe_new_log("    RIGHT: %s", PRINT_PRESSED(right));
        GFraMe_new_log("     A: %s", PRINT_PRESSED(a));
        GFraMe_new_log("     B: %s", PRINT_PRESSED(b));
        GFraMe_new_log("     X: %s", PRINT_PRESSED(x));
        GFraMe_new_log("     Y: %s", PRINT_PRESSED(y));
        GFraMe_new_log("    R1: %s", PRINT_PRESSED(r1));
        GFraMe_new_log("    R2: %s", PRINT_PRESSED(r2));
        GFraMe_new_log("    R3: %s", PRINT_PRESSED(r3));
        GFraMe_new_log("    L1: %s", PRINT_PRESSED(l1));
        GFraMe_new_log("    L2: %s", PRINT_PRESSED(l2));
        GFraMe_new_log("    L3: %s", PRINT_PRESSED(l3));
        GFraMe_new_log("     START: %s", PRINT_PRESSED(start));
        GFraMe_new_log("    SELECT: %s", PRINT_PRESSED(select));
        GFraMe_new_log("      HOME: %s", PRINT_PRESSED(home));
        i++;
    }
    GFraMe_new_log("=========================================");
}

