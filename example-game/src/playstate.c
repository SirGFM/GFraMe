/**
 * @src/playstate.c
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "enemies.h"
#include "global.h"
#include "player.h"
#include "multiplier.h"
#include "score.h"
#include "playstate.h"

// Define some variables needed by the events module
GFraMe_event_setup();

//====================================================//
// Local (to this state) variable (sprites and stuff) //
//====================================================//
/**
 * Timer used to spawn stuff
 */
GFraMe_accumulator acc_timer;
/**
 * Background
 */
GFraMe_tilemap bg;
/**
 * Floor (image and for collision [when implemented])
 */
GFraMe_tilemap floor_tm;
#define	BG_W	40
#define BG_H	10
/**
 * Tilemap data for the background
 */
char bg_data[BG_W * BG_H];
#define	F_W	20
#define	F_H	6
#define F_Y	144
/**
 * Tilemap data for the floor
 */
char floor_data[F_W * F_H];
/**
 * Floor collideable; doesn't need gfx
 */
GFraMe_object ground;
/**
 * Whether the intro screen has given place to the actual playstate
 */
int game_init = 0;
/**
 * Whether the game should restart when it leaves the loop
 */
int restart = 0;

static GFraMe_tilemap init_text;
static char init_data[20];
static void init_eh();
static void init_update();
static void init_draw();

/**
 * Initializes the play state (i.e., reset positions and stuff)
 */
void ps_init();
/**
 * Check for any event on the queue and handle it
 */
void ps_event_handler();
/**
 * Updates everything on the state
 */
void ps_do_update();
/**
 * Renders the state
 */
void ps_do_draw();
/**
 *
 */
void ps_on_click(int X, int Y);
/**
 * Called before exiting the loop to clean up any allocated resource.
 */
void ps_cleanup();

/**
 * Initialize and run the play state loop
 */
void ps_loop() {
_begin:
	ps_init();
	while (gl_running && !game_init) {
		init_eh();
		init_update();
		init_draw();
	}
	while (gl_running) {
		ps_event_handler();
		ps_do_update();
		ps_do_draw();
	}
	// TODO display stats
	// I don't care what you say, this isn't bad design!
	if (restart)
		goto _begin;
	ps_cleanup();
}

#define BASE_BG_TILE	32
#define BASE_FLOOR_TILE	21
void ps_init() {
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
	while (i < F_W) {
		floor_data[i] = BASE_FLOOR_TILE + i % 2;
		i++;
	}
	// Second row
	while (i < F_W*2) {
		floor_data[i] = BASE_FLOOR_TILE + i % 2 + 2;
		i++;
	}
	// Every other row
	while (i < F_W*F_H) {
		floor_data[i] = BASE_FLOOR_TILE + 4;
		if (i / F_W % 2 == 0)
			floor_data[i] += i % 2 == 0;
		else
			floor_data[i] += i % 2 == 1;
		i++;
	}
	// Initialize the player
	player_init();
	// Initialize the tilemap
	GFraMe_tilemap_init(&bg, BG_W, BG_H, bg_data, &gl_sset8x16, NULL, 0);
	// Initialize the tilemap
	GFraMe_tilemap_init(&floor_tm, F_W, F_H, floor_data, &gl_sset16, NULL, 0);
	floor_tm.y = F_Y;
	// Create the floor for collisions
	GFraMe_object_clear(&ground);
	GFraMe_object_set_y(&ground, 160);
	GFraMe_object_set_hitbox(&ground, GFraMe_set_hitbox_upper_left,
							 0, 0, GFraMe_buffer_w, 16);
	// Initialize every enemy as non existing
	enemies_init();
	// Initialize the score subsystem
	score_init();
	// Initialize the multiplier subsystem
	multi_init();
	// Initialize the spawn timer
	GFraMe_accumulator_init_fps(&acc_timer, 1, 1);
	// Initialize the timer and clean the events accumulated on the queue
	GFraMe_event_init(60, 60);
	// Clean restart flag
	restart = 0;
	game_init = 0;
	// Set init text
	GFraMe_tilemap_init(&init_text, 20, 1, init_data, &gl_sset8, NULL, 0);
	init_text.x = (320 - 20*8) / 2;
	init_text.y = 240 - 32 - 12;
	i = 0;
	init_data[i++] = CHAR2TILE('-');
	init_data[i++] = CHAR2TILE('-');
	init_data[i++] = CHAR2TILE(' ');
#ifndef MOBILE
	init_data[i++] = CHAR2TILE('C');
	init_data[i++] = CHAR2TILE('L');
	init_data[i++] = CHAR2TILE('I');
	init_data[i++] = CHAR2TILE('C');
	init_data[i++] = CHAR2TILE('K');
#else
	init_data[i++] = CHAR2TILE('T');
	init_data[i++] = CHAR2TILE('O');
	init_data[i++] = CHAR2TILE('U');
	init_data[i++] = CHAR2TILE('C');
	init_data[i++] = CHAR2TILE('H');
#endif
	init_data[i++] = CHAR2TILE(' ');
	init_data[i++] = CHAR2TILE('T');
	init_data[i++] = CHAR2TILE('O');
	init_data[i++] = CHAR2TILE(' ');
	init_data[i++] = CHAR2TILE('S');
	init_data[i++] = CHAR2TILE('T');
	init_data[i++] = CHAR2TILE('A');
	init_data[i++] = CHAR2TILE('R');
	init_data[i++] = CHAR2TILE('T');
	init_data[i++] = CHAR2TILE(' ');
	init_data[i++] = CHAR2TILE('-');
	init_data[i++] = CHAR2TILE('-');
}

void ps_event_handler() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
			GFraMe_accumulator_update(&acc_timer, __dt__);
		GFraMe_event_on_mouse_down();
			ps_on_click(GFraMe_event_mouse_x-8, GFraMe_event_mouse_y-8);
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}

void ps_do_update() {
	int i;
	GFraMe_object *pl;
	GFraMe_object *en;
	
	GFraMe_event_update_begin();
		if (GFraMe_accumulator_loop(&acc_timer)) {
			int new_time = enemies_do_spawn();
			GFraMe_accumulator_init_set(&acc_timer, new_time, new_time);
		}
		// Check if the player is near the appex, and should slowdown
		if (player_slowdown())
			GFraMe_event_elapsed >>= 2;
		// Then, update the player
		player_update(GFraMe_event_elapsed);
		// Collide the player with the floor
		if (GFraMe_object_overlap(&ground, player_get_object(),
								  GFraMe_first_fixed) == GFraMe_ret_ok)
			player_on_ground();
		// Update every enemy
		enemies_update(GFraMe_event_elapsed);
		// Collide every enemy with the player
		pl = player_get_object();
		i = -1;
		while (1) {
			en = enemies_get_object(++i);
			if (!en)
				break;
			else if (!enemies_is_alive(i))
				continue;
			if (GFraMe_object_overlap(en, pl, GFraMe_dont_collide)
				== GFraMe_ret_ok) {
				if (player_on_squash() == GFraMe_ret_ok) {
					enemies_on_hit(i);
				}
				else {
					// TODO Do something on collision
				}
			}
		}
		// Update the current displaying score
		score_update(GFraMe_event_elapsed);
		// Update the current multiplier
		multi_update(GFraMe_event_elapsed);
	GFraMe_event_update_end();
}

void ps_do_draw() {
	GFraMe_event_draw_begin();
		// Draw the tilemap
		GFraMe_tilemap_draw(&bg);
		GFraMe_tilemap_draw(&floor_tm);
		// Draw every enemy
		enemies_draw();
		// Draw the player
		player_draw();
		// Draw the current score
		score_draw();
		// Draw the current multiplier
		multi_draw();
	GFraMe_event_draw_end();
}

void ps_on_click(int X, int Y) {
	if (player_jump(X) == GFraMe_ret_failed)
		player_set_target(X, Y);
}

void ps_cleanup() {
	GFraMe_tilemap_clear(&bg);
}

static void init_eh() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
			GFraMe_accumulator_update(&acc_timer, __dt__);
		GFraMe_event_on_mouse_down();
			game_init = 1;
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}
static void init_update() {
	GFraMe_event_update_begin();
	GFraMe_event_update_end();
}
static void init_draw() {
	GFraMe_event_draw_begin();
		// Draw the tilemap
		GFraMe_tilemap_draw(&bg);
		GFraMe_tilemap_draw(&floor_tm);
		// Draw start message
		GFraMe_tilemap_draw(&init_text);
	GFraMe_event_draw_end();
}

