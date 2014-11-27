/**
 * @file include/GFraMe/GFraMe_key.h
 */
#ifndef __GFRAME_KEY_H_
#define __GFRAME_KEY_H_

#include <SDL2/SDL_events.h>

struct stGFraMe_key {
    char a :1;
    char b :1;
    char c :1;
    char d :1;
    char e :1;
    char f :1;
    char g :1;
    char h :1;
    char i :1;
    char j :1;
    char k :1;
    char l :1;
    char m :1;
    char n :1;
    char o :1;
    char p :1;
    char q :1;
    char r :1;
    char s :1;
    char t :1;
    char u :1;
    char v :1;
    char w :1;
    char x :1;
    char y :1;
    char z :1;
    char one :1;
    char two :1;
    char three :1;
    char four :1;
    char five :1;
    char six :1;
    char seven :1;
    char eigth :1;
    char nine :1;
    char zero :1;
    char n1 :1;
    char n2 :1;
    char n3 :1;
    char n4 :1;
    char n5 :1;
    char n6 :1;
    char n7 :1;
    char n8 :1;
    char n9 :1;
    char n0 :1;
    //char nequals :1;
    char nplus :1;
    char nminus :1;
    char ndivide :1;
    char nmult :1;
    char nperiod :1;
    char ncomma :1;
    char nenter :1;
    char minus :1;
    //char plus :1;
    char equals :1;
    char comma :1;
    char period :1;
    char lctrl :1;
    char lalt :1;
    char lshift :1;
    char rctrl :1;
    char ralt :1;
    char rshift :1;
    char tab :1;
    char space :1;
    char enter :1;
    char backspace :1;
    char backslash :1; // '\'
    char slash :1;    // '/'
    char esc :1;
    char up :1;
    char down :1;
    char left :1;
    char right :1;
    char insert :1;
    char delete :1;
    char home :1;
    char end :1;
    char pgup :1;
    char pgdown :1;
    char f1 :1;
    char f2 :1;
    char f3 :1;
    char f4 :1;
    char f5 :1;
    char f6 :1;
    char f7 :1;
    char f8 :1;
    char f9 :1;
    char f10 :1;
    char f11 :1;
    char f12 :1;
};

extern struct stGFraMe_key GFraMe_keys;

void GFraMe_key_init();
void GFraMe_key_upd(SDL_Event *event, char set);

#endif

