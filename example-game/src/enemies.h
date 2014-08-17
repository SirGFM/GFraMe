/**
 * @src/enemies.h
 */
#ifndef __ENEMIES_H_
#define __ENEMIES_H_

#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>

void enemies_spawn_random(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_update(GFraMe_sprite *en, int ms);
GFraMe_Object *enemies_get_object(int i);

#endif

