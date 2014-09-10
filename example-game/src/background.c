/**
 * src/background.c
 */
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_tilemap.h>
#include "global.h"
#include "background.h"

#define	BG_W	40
#define BG_H	10
#define	FLOOR_W	20
#define	FLOOR_H	6
#define FLOOR_Y	144
#define BASE_BG_TILE	32
#define BASE_FLOOR_TILE	21

/**
 * Background
 */
static GFraMe_tilemap bg;
/**
 * Floor (image and for collision [when implemented])
 */
static GFraMe_tilemap floor_tm;
/**
 * Tilemap data for the background
 */
static char bg_data[BG_W * BG_H];
/**
 * Tilemap data for the floor
 */
static char floor_data[FLOOR_W * FLOOR_H];
/**
 * Floor collideable; doesn't need gfx
 */
static GFraMe_object ground;

void background_init() {
	int i;
	i = 0;
	// Fill up the background with tiles
	while (i < BG_W*5) {
		bg_data[i] = BASE_BG_TILE + i / BG_W;
		i++;
	}
	while (i < BG_W * 6) {
		bg_data[i] = BASE_BG_TILE + i / BG_W - 1;
		i++;
	}
	while (i < BG_W*BG_H) {
		bg_data[i] = BASE_BG_TILE + i / BG_W - 1;
		i++;
	}
	// Fill up the floor tiles
	i = 0;
	// Top row
	while (i < FLOOR_W) {
		floor_data[i] = BASE_FLOOR_TILE + i % 2;
		i++;
	}
	// Second row
	while (i < FLOOR_W*2) {
		floor_data[i] = BASE_FLOOR_TILE + i % 2 + 2;
		i++;
	}
	// Every other row
	while (i < FLOOR_W*FLOOR_H) {
		floor_data[i] = BASE_FLOOR_TILE + 4;
		if (i / FLOOR_W % 2 == 0)
			floor_data[i] += i % 2 == 0;
		else
			floor_data[i] += i % 2 == 1;
		i++;
	}
	// Initialize the tilemap
	GFraMe_tilemap_init(&bg, BG_W, BG_H, bg_data, &gl_sset8x16, NULL, 0);
	// Initialize the tilemap
	GFraMe_tilemap_init(&floor_tm, FLOOR_W, FLOOR_H, floor_data,
						&gl_sset16, NULL, 0);
	floor_tm.y = FLOOR_Y;
	// Create the floor for collisions
	GFraMe_object_clear(&ground);
	GFraMe_object_set_y(&ground, 160);
	GFraMe_hitbox_set(GFraMe_object_get_hitbox(&ground),
					  GFraMe_hitbox_upper_left, 0, 0, GFraMe_screen_w, 16);
}

void background_draw() {
	GFraMe_tilemap_draw(&bg);
	GFraMe_tilemap_draw(&floor_tm);
}

GFraMe_object* background_get_object() {
	return &ground;
}

