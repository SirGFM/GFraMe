/**
 * @src/enemies.c
 */
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_util.h>
#include "global.h"
#include "enemies.h"

void enemies_bug_easy_init(GFraMe_sprite *en);
void enemies_bug_norm_init(GFraMe_sprite *en);
void enemies_bug_hard_init(GFraMe_sprite *en);

/**
 * Simple macro to create an sprite that uses the 16x16 spriteset.
 */
#define INIT_SPRITE16(spr, h) \
	GFraMe_sprite_init(spr, -16, 160-h, 16, h, &gl_sset16, 0, h-16); \

void enemies_spawn_random(GFraMe_sprite *en) {
	int r = GFraMe_util_randomi() % 10;
	switch(r) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			enemies_bug_easy_init(en);
		break;
		case 6:
		case 7:
		case 8:
			enemies_bug_norm_init(en);
		break;
		case 9:
			enemies_bug_hard_init(en);
	}
}

void enemies_bug_easy_init(GFraMe_sprite *en) {
	INIT_SPRITE16(en, 9);
	en->id = 1;
	en->cur_tile = 11;
	en->obj.vx = 80;
}

void enemies_bug_norm_init(GFraMe_sprite *en) {
	INIT_SPRITE16(en, 9);
	en->id = 2;
	en->cur_tile = 12;
	en->obj.vx = 115;
}

void enemies_bug_hard_init(GFraMe_sprite *en) {
	INIT_SPRITE16(en, 9);
	en->id = 3;
	en->cur_tile = 13;
	en->obj.vx = 125;
}

