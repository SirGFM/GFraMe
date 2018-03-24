/**
 * @file src/gframe_key.c
 */
#include <GFraMe/GFraMe_keys.h>
#include <SDL2/SDL_events.h>
#include <string.h>

struct stGFraMe_key GFraMe_keys;

void GFraMe_key_init() {
    memset(&GFraMe_keys, 0x0, sizeof(GFraMe_keys));
}

void GFraMe_key_upd(SDL_Event *event, char set) {
    switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_A:           GFraMe_keys.a = set; break;
        case SDL_SCANCODE_B:           GFraMe_keys.b = set; break;
        case SDL_SCANCODE_C:           GFraMe_keys.c = set; break;
        case SDL_SCANCODE_D:           GFraMe_keys.d = set; break;
        case SDL_SCANCODE_E:           GFraMe_keys.e = set; break;
        case SDL_SCANCODE_F:           GFraMe_keys.f = set; break;
        case SDL_SCANCODE_G:           GFraMe_keys.g = set; break;
        case SDL_SCANCODE_H:           GFraMe_keys.h = set; break;
        case SDL_SCANCODE_I:           GFraMe_keys.i = set; break;
        case SDL_SCANCODE_J:           GFraMe_keys.j = set; break;
        case SDL_SCANCODE_K:           GFraMe_keys.k = set; break;
        case SDL_SCANCODE_L:           GFraMe_keys.l = set; break;
        case SDL_SCANCODE_M:           GFraMe_keys.m = set; break;
        case SDL_SCANCODE_N:           GFraMe_keys.n = set; break;
        case SDL_SCANCODE_O:           GFraMe_keys.o = set; break;
        case SDL_SCANCODE_P:           GFraMe_keys.p = set; break;
        case SDL_SCANCODE_Q:           GFraMe_keys.q = set; break;
        case SDL_SCANCODE_R:           GFraMe_keys.r = set; break;
        case SDL_SCANCODE_S:           GFraMe_keys.s = set; break;
        case SDL_SCANCODE_T:           GFraMe_keys.t = set; break;
        case SDL_SCANCODE_U:           GFraMe_keys.u = set; break;
        case SDL_SCANCODE_V:           GFraMe_keys.v = set; break;
        case SDL_SCANCODE_W:           GFraMe_keys.w = set; break;
        case SDL_SCANCODE_X:           GFraMe_keys.x = set; break;
        case SDL_SCANCODE_Y:           GFraMe_keys.y = set; break;
        case SDL_SCANCODE_Z:           GFraMe_keys.z = set; break;
        case SDL_SCANCODE_1:           GFraMe_keys.one = set; break;
        case SDL_SCANCODE_2:           GFraMe_keys.two = set; break;
        case SDL_SCANCODE_3:           GFraMe_keys.three = set; break;
        case SDL_SCANCODE_4:           GFraMe_keys.four = set; break;
        case SDL_SCANCODE_5:           GFraMe_keys.five = set; break;
        case SDL_SCANCODE_6:           GFraMe_keys.six = set; break;
        case SDL_SCANCODE_7:           GFraMe_keys.seven = set; break;
        case SDL_SCANCODE_8:           GFraMe_keys.eigth = set; break;
        case SDL_SCANCODE_9:           GFraMe_keys.nine = set; break;
        case SDL_SCANCODE_0:           GFraMe_keys.zero = set; break;
        case SDL_SCANCODE_KP_1:        GFraMe_keys.n1 = set; break;
        case SDL_SCANCODE_KP_2:        GFraMe_keys.n2 = set; break;
        case SDL_SCANCODE_KP_3:        GFraMe_keys.n3 = set; break;
        case SDL_SCANCODE_KP_4:        GFraMe_keys.n4 = set; break;
        case SDL_SCANCODE_KP_5:        GFraMe_keys.n5 = set; break;
        case SDL_SCANCODE_KP_6:        GFraMe_keys.n6 = set; break;
        case SDL_SCANCODE_KP_7:        GFraMe_keys.n7 = set; break;
        case SDL_SCANCODE_KP_8:        GFraMe_keys.n8 = set; break;
        case SDL_SCANCODE_KP_9:        GFraMe_keys.n9 = set; break;
        case SDL_SCANCODE_KP_0:        GFraMe_keys.n0 = set; break;
        //case SDL_nequals: GFraMe_keys.nequals = set; break;
        case SDL_SCANCODE_KP_PLUS:     GFraMe_keys.nplus = set; break;
        case SDL_SCANCODE_KP_MINUS:    GFraMe_keys.minus = set; break;
        case SDL_SCANCODE_KP_DIVIDE:   GFraMe_keys.ndivide = set; break;
        case SDL_SCANCODE_KP_MULTIPLY: GFraMe_keys.nmult = set; break;
        case SDL_SCANCODE_KP_PERIOD:   GFraMe_keys.nperiod = set; break;
        case SDL_SCANCODE_KP_COMMA:    GFraMe_keys.ncomma = set; break;
        case SDL_SCANCODE_KP_ENTER:    GFraMe_keys.nenter = set; break;
        case SDL_SCANCODE_MINUS:       GFraMe_keys.minus = set; break;
        //case SDL_plus: GFraMe_keys.plus = set; break;
        case SDL_SCANCODE_EQUALS:      GFraMe_keys.equals = set; break;
        case SDL_SCANCODE_COMMA:       GFraMe_keys.comma = set; break;
        case SDL_SCANCODE_PERIOD:      GFraMe_keys.period = set; break;
        case SDL_SCANCODE_LCTRL:       GFraMe_keys.lctrl = set; break;
        case SDL_SCANCODE_LALT:        GFraMe_keys.lalt = set; break;
        case SDL_SCANCODE_LSHIFT:      GFraMe_keys.lshift = set; break;
        case SDL_SCANCODE_RCTRL:       GFraMe_keys.rctrl = set; break;
        case SDL_SCANCODE_RALT:        GFraMe_keys.ralt = set; break;
        case SDL_SCANCODE_RSHIFT:      GFraMe_keys.rshift = set; break;
        case SDL_SCANCODE_TAB:         GFraMe_keys.tab = set; break;
        case SDL_SCANCODE_SPACE:       GFraMe_keys.space = set; break;
        case SDL_SCANCODE_RETURN:      GFraMe_keys.enter = set; break;
        case SDL_SCANCODE_BACKSPACE:   GFraMe_keys.backspace = set; break;
        case SDL_SCANCODE_BACKSLASH:   GFraMe_keys.backslash = set; break;
        case SDL_SCANCODE_SLASH:       GFraMe_keys.slash = set; break;
        case SDL_SCANCODE_ESCAPE:      GFraMe_keys.esc = set; break;
        case SDL_SCANCODE_UP:          GFraMe_keys.up = set; break;
        case SDL_SCANCODE_DOWN:        GFraMe_keys.down = set; break;
        case SDL_SCANCODE_LEFT:        GFraMe_keys.left = set; break;
        case SDL_SCANCODE_RIGHT:       GFraMe_keys.right = set; break;
        case SDL_SCANCODE_INSERT:      GFraMe_keys.insert = set; break;
        case SDL_SCANCODE_DELETE:      GFraMe_keys.delete = set; break;
        case SDL_SCANCODE_HOME:        GFraMe_keys.home = set; break;
        case SDL_SCANCODE_END:         GFraMe_keys.end = set; break;
        case SDL_SCANCODE_PAGEUP:      GFraMe_keys.pgup = set; break;
        case SDL_SCANCODE_PAGEDOWN:    GFraMe_keys.pgdown = set; break;
        case SDL_SCANCODE_F1:          GFraMe_keys.f1 = set; break;
        case SDL_SCANCODE_F2:          GFraMe_keys.f2 = set; break;
        case SDL_SCANCODE_F3:          GFraMe_keys.f3 = set; break;
        case SDL_SCANCODE_F4:          GFraMe_keys.f4 = set; break;
        case SDL_SCANCODE_F5:          GFraMe_keys.f5 = set; break;
        case SDL_SCANCODE_F6:          GFraMe_keys.f6 = set; break;
        case SDL_SCANCODE_F7:          GFraMe_keys.f7 = set; break;
        case SDL_SCANCODE_F8:          GFraMe_keys.f8 = set; break;
        case SDL_SCANCODE_F9:          GFraMe_keys.f9 = set; break;
        case SDL_SCANCODE_F10:         GFraMe_keys.f10 = set; break;
        case SDL_SCANCODE_F11:         GFraMe_keys.f11 = set; break;
        case SDL_SCANCODE_F12:         GFraMe_keys.f12 = set; break;
        default:{}
    }
}

