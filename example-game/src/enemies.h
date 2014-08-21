/**
 * @src/enemies.h
 */
#ifndef __ENEMIES_H_
#define __ENEMIES_H_

#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>

void enemies_init();
void enemies_update(int ms);
void enemies_draw();
GFraMe_object *enemies_get_object(int i);
int enemies_is_alive(int i);
void enemies_on_hit(int i);
int enemies_do_spawn();
void enemies_spawn_random(GFraMe_sprite *en, GFraMe_animation *anim);

#endif

