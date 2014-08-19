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

static int bug01_anim_data[2] = {15, 16};
static int bug02_anim_data[2] = {18, 19};
static int bug03_anim_data[2] = {21, 22};
static int beetle01_anim_data[2] = {24, 25};
static int beetle02_anim_data[2] = {32, 33};

/**
 * Array for every enemy (possibly) on the screen
 */
GFraMe_sprite enemies[MAX_ENEMIES];
/**
 * Array with on animation for each enemy
 */
GFraMe_animation en_animation[MAX_ENEMIES];
/**
 * Count for how many frames an enemy should be stunned after hit
 */
int stop_frames[MAX_ENEMIES];

static void enemies_kill(int i);

void enemies_bug_easy_init(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_bug_norm_init(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_bug_hard_init(GFraMe_sprite *en, GFraMe_animation *anim);

void enemies_beetle_easy_init(GFraMe_sprite *en, GFraMe_animation *anim);
void enemies_beetle_norm_init(GFraMe_sprite *en, GFraMe_animation *anim);

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
		enemies_kill(i);
		i++;
	}
}

void enemies_update(int ms) {
	int i;
	i = 0;
	while (i < MAX_ENEMIES) {
		if (enemies[i].is_active) {
			GFraMe_sprite *en = enemies + i;
			GFraMe_sprite_update(en, ms);
			if (en->obj.x > 320) {
				en->id = 0;
				en->is_active = 0;
				en->is_visible = 0;
				i++;
				continue;
			}
		}
		else if (stop_frames[i] > 0) {
			stop_frames[i]--;
			if (stop_frames[i] == 0) {
				enemies[i].offset_y -= 4;
				enemies[i].is_active = 1;
			}
		}
		i++;
	}
}

void enemies_draw() {
	int i;
	i = 0;
	while (i < MAX_ENEMIES) {
		if (enemies[i].is_visible)
			GFraMe_sprite_draw(enemies + i);
		i++;
	}
}

GFraMe_object *enemies_get_object(int i) {
	if (i >= MAX_ENEMIES)
		return NULL;
	return &enemies[i].obj;
}

void enemies_on_hit(int i) {
	enemies[i].hp--;
	if (enemies[i].hp <= 0)
		enemies_kill(i);
	else {
		stop_frames[i] = 4;
		enemies[i].offset_y += 4;
		enemies[i].is_active = 0;
		switch (enemies[i].id) {
			case 1: enemies[i].cur_tile = 17; break;
			case 2: enemies[i].cur_tile = 20; break;
			case 3: enemies[i].cur_tile = 23; break;
			case 4: enemies[i].cur_tile = 31; break;
			case 5: enemies[i].cur_tile = 39; break;
		}
	}
}

static void enemies_kill(int i) {
	enemies[i].id = 0;
	enemies[i].is_active = 0;
	enemies[i].is_visible = 0;
	enemies[i].obj.x = 640;
	stop_frames[i] = 0;
}

int enemies_do_spawn() {
	int i;
	int time = 1000;
	i = 0;
	while (i < MAX_ENEMIES) {
		if (enemies[i].id == 0) {
			enemies_spawn_random(enemies+i, en_animation+i);
			break;
		}
		i++;
	}
	// Sets a new spawn time, depending on what was spawned
	if (i < MAX_ENEMIES) {
		// r = [-500, 500], with a step of 100
		time = (GFraMe_util_randomi() % 11 - 5) * 100;
		// easier enemies spawn faster! (also, 1s + r)
		time = (1000 + time) / (3 - (enemies[i].id - 1) % 3);
	}
	return time;
}

void enemies_spawn_random(GFraMe_sprite *en, GFraMe_animation *anim) {
	int r = GFraMe_util_randomi() % 10;
	switch(r) {
		case 0:
		case 1:
		case 2:
			enemies_bug_easy_init(en, anim);
		break;
		case 3:
		case 4:
		case 5:
			enemies_bug_norm_init(en, anim);
		break;
		case 6:
			enemies_bug_hard_init(en, anim);
		break;
		case 7:
		case 8:
			enemies_beetle_easy_init(en, anim);
		break;
		case 9:
			enemies_beetle_norm_init(en, anim);
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
	en->id = 4;
	en->hp = 3;
	en->obj.vx = 100;
	en->offset_y += 8;
}

void enemies_beetle_norm_init(GFraMe_sprite *en, GFraMe_animation *anim) {
	INIT_SPRITE32(en, 17, anim, 8, beetle02_anim_data, 2);
	en->id = 5;
	en->hp = 5;
	en->obj.vx = 120;
	en->offset_y += 8;
}

