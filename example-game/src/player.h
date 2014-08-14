/**
 * @src/player.h
 */
#ifndef __PLAYER_H_
#define __PLAYER_H_

#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_object.h>

void player_init();
void player_update(int ms);
void player_draw();
int player_slowdown();
void player_on_ground();
GFraMe_ret player_on_squash();
void player_set_target(int X, int Y);
GFraMe_ret player_jump(int X);
GFraMe_object *player_get_object();

#endif

