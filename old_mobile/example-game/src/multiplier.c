/**
 * @src/multiplier.h
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_tilemap.h>
#include "global.h"
#include "player.h"
#include "multiplier.h"

/**
 * Tilemap used to display the multiplier
 */
static GFraMe_tilemap tm_multi;
/**
 * Current data used by the multiplier
 */
static char data[5];
/**
 * Timer used to decrease the current level
 */
static int timer;
/**
 * Current time being decreased (to calculated percentagem)
 */
static int max_time;
/**
 * Used to simplify getting/setting the current value
 */
static int multi;

static void multi_set_data();
static void multi_set_data();
static void multi_set_time();

void multi_init() {
	// Feed initial (constant) data into the buffer
	data[1] = CHAR2TILE('.');
	data[3] = CHAR2TILE(' ');
	data[4] = CHAR2TILE('X');
	// Initialize the tilemap and its position
	GFraMe_tilemap_init(&tm_multi, 5, 1, data, &gl_sset8, NULL, 0);
	tm_multi.x = 8;
	tm_multi.y = 8;
	// Initialize the timer and multiplier
	multi = 10;
	timer = 0;
	max_time = 0;
	// Set current data
	multi_set_data();
}

void multi_update(int ms) {
	//double alpha = 1.0;
	GFraMe_object *pl;
	// Update the timer
	if (timer > 0) {
		timer -= ms;
		if (timer == 0 && multi > 10) {
			multi--;
			multi_set_time();
			multi_set_data();
		}
		// Get how long it has passed, in percentage
		//if (max_time > 0)
		//	alpha = ((double)timer) / (double)max_time;
	}
	// Set the multiplier position above the player
	pl = player_get_object();
	tm_multi.x = pl->x + ((int)(pl->hitbox.hw * 2)) + 8;
	tm_multi.y = pl->y - 16;
	// Set the scale to the percentage
	//tm_multi.sx = alpha;
	//tm_multi.sy = alpha;
}

GFraMe_ret multi_draw() {
	return GFraMe_tilemap_draw(&tm_multi);
}

double multi_get() {
	return ((double)multi) / 10.0;
}

void multi_inc() {
	// Increase the multiplier (but cap its value)
	if (multi < 99) {
		multi++;
		// Set current data
		multi_set_data();
	}
	// Set how long until it decreases
	multi_set_time();
}

void multi_half() {
	// Halve the current value (keeping the minimum at 1.0X)
	multi = multi / 2 + 5;
	// Set how long until it decreases
	multi_set_time();
	// Set current data
	multi_set_data();
}

void multi_reset() {
	// Reset the multiplier and the timer
	multi = 10;
	timer = 0;
	max_time = 0;
	// Set current data
	multi_set_data();
}

static void multi_set_data() {
	data[0] = CHAR2TILE((multi / 10 + '0'));
	data[2] = CHAR2TILE((multi % 10 + '0'));
}

static void multi_set_time() {
	if (multi < 50)
		timer = (60 - multi) * 1000;
	else
		timer = 1000;
	max_time = timer;
}

