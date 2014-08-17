/**
 * @src/enemies.c
 */
#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_util.h>
#include <stdio.h>
#include "global.h"
#include "player.h"
#include "enemies.h"

#define MAX_ENEMIES	32

static int bug01_anim_data[2] = {4, 5};
static int bug02_anim_data[2] = {6, 7};
static int bug03_anim_data[2] = {8, 9};
static int beetle01_anim_data[2] = {10, 11};

/**
 * Array for every enemy (possibly) on the screen
 */
GFraMe_sprite enemies[MAX_ENEMIES];
/**
 * Array with on animation for each enemy
 */
GFraMe_animation en_animation[MAX_ENEMIES];

void enemies_bug_easy_init(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_bug_norm_init(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_bug_hard_init(GFraMe_sprite *en, GFraMe_animation *anim);

void enemies_beetle_easy_init(GFraMe_sprite *en, GFraMe_animation *anim);

/**
 * Simple macro to create an sprite that uses the 16x16 spriteset.
 */
#define INIT_SPRITE16(spr, h, an, an_fps, an_data, an_len) \
	GFraMe_sprite_init(spr, -16, 160-h, 16, h, &gl_sset16, 0, h-16); \
	GFraMe_animation_init(an, an_fps, an_data, an_len, 1); \
	GFraMe_sprite_set_animation(spr, an)
/**
 * Simple macro to create an sprite that uses the 16x16 spriteset.
 */
#define INIT_SPRITE32(spr, h, an, an_fps, an_data, an_len) \
	GFraMe_sprite_init(spr, -32, 160-h, 32, h, &gl_sset32, 0, h/2-16); \
	GFraMe_animation_init(an, an_fps, an_data, an_len, 1); \
	GFraMe_sprite_set_animation(spr, an)

void enemies_init() {
	int i = 0;
	while (i < MAX_ENEMIES) {
		enemies[i].is_active = 0;
		enemies[i].is_visible = 0;
		i++;
	}
}

void enemies_update(GFraMe_sprite *en, int ms) {
	int i;
	i = 0;
	while (i < MAX_ENEMIES) {
		if (enemies[i].is_active) {
			GFraMe_sprite *en = enemies + i;
			GFraMe_sprite_update(en, GFraMe_event_elapsed);
			if (en->obj.x > 320) {
				en->id = 0;
				en->is_active = 0;
				en->is_visible = 0;
				i++;
				continue;
			}
		}
		i++;
	}
}

GFraMe_Object *enemies_get_object(int i) {
	if (i >= MAX_ENEMIES)
		return NULL;
	return &enemies[i].obj;
}

void enemies_spawn_random(GFraMe_sprite *en, GFraMe_animation *anim) {
	int r = GFraMe_util_randomi() % 10;
	switch(r) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			enemies_bug_easy_init(en, anim);
		break;
		case 6:
		case 7:
		case 8:
		//case 9:
			enemies_bug_norm_init(en, anim);
			//enemies_beetle_easy_init(en, anim);
		break;
		case 9:
			enemies_bug_hard_init(en, anim);
	}
}

void enemies_bug_easy_init(GFraMe_sprite *en, GFraMe_animation *anim) {
	INIT_SPRITE32(en, 13, anim, 8, bug01_anim_data, 2);
	en->id = 1;
	en->hp = 1;
	en->obj.vx = 80;
}

void enemies_bug_norm_init(GFraMe_sprite *en, GFraMe_animation *anim) {
	INIT_SPRITE32(en, 13, anim, 8, bug02_anim_data, 2);
	en->id = 2;
	en->hp = 2;
	en->obj.vx = 115;
}

void enemies_bug_hard_init(GFraMe_sprite *en, GFraMe_animation *anim) {
	INIT_SPRITE32(en, 13, anim, 8, bug03_anim_data, 2);
	en->id = 3;
	en->hp = 3;
	en->obj.vx = 125;
}

void enemies_beetle_easy_init(GFraMe_sprite *en, GFraMe_animation *anim) {
	INIT_SPRITE32(en, 17, anim, 8, beetle01_anim_data, 2);
	en->id = 7;
	en->hp = 3;
	en->cur_tile = 10;
	en->obj.vx = 100;
}

