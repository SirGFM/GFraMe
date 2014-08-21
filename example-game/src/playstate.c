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
 * Background (image and for collision [when implemented])
 */
GFraMe_tilemap bg;
/**
 * Tilemap data for the background
 */
char bg_data[20*15];
/**
 * Floor collideable; doesn't need gfx
 */
GFraMe_object ground;

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
	ps_init();
	while (gl_running) {
		ps_event_handler();
		ps_do_update();
		ps_do_draw();
	}
	ps_cleanup();
}

#define BASE_TILE	16
void ps_init() {
	int i;
	i = 0;
	// Fill up the background with tiles
	while (i < 20*10) {
		int rng = GFraMe_util_randomi() % 10 < 8; // 80% = 1; 20% = 0;
		int row = i / 20 % 2 == 0; // even row = 1; odd row = 0;
		int col = i % 2 == 1; // even column = 0; odd column = 1;
		bg_data[i] = BASE_TILE+6  // The base BG tile
					+rng*2		  // Select from two type of BG
					+(row == col);// Make a checkered board
		i++;
	}
	// Fill the floor tiles
	// Fill the first row with a specific type of tiles
	while (i < 20*11) {
		bg_data[i] = BASE_TILE + (i % 2 == 0);
		i++;
	}
	// Fill the rest with random data
	while (i < 20*15) {
		int rng = GFraMe_util_randomi() % 10 < 8; // 80% = 1; 20% = 0;
		int row = i / 20 % 2 == 0; // even row = 1; odd row = 0;
		int col = i % 2 == 0; // even column = 1; odd = 0;
		bg_data[i] = BASE_TILE + 2 // The base floor tile
					+(row == col)
					+(rng)*2;
		i++;
	}
	// Initialize the player
	player_init();
	// Initialize the tilemap
	GFraMe_tilemap_init(&bg, 20, 15, bg_data, &gl_sset16, NULL, 0);
	// Create the floor for collisions
	GFraMe_object_clear(&ground);
	GFraMe_object_set_y(&ground, 160);
	GFraMe_object_set_hitbox(&ground, GFraMe_set_hitbox_upper_left,
							 0, 0, GFraMe_buffer_w, 16);
	// Initialize every enemy as non existing
	enemies_init();
	// Initialize the score subsystem
	score_init();
	// Initialize the spawn timer
	GFraMe_accumulator_init_fps(&acc_timer, 1, 1);
	// Initialize the timer and clean the events accumulated on the queue
	GFraMe_event_init(60, 60);
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
	GFraMe_event_update_end();
}

void ps_do_draw() {
	GFraMe_event_draw_begin();
		// Draw the tilemap
		GFraMe_tilemap_draw(&bg);
		// Draw every enemy
		enemies_draw();
		// Draw the player
		player_draw();
		// Draw the current score
		score_draw();
	GFraMe_event_draw_end();
}

void ps_on_click(int X, int Y) {
	if (player_jump(X) == GFraMe_ret_failed)
		player_set_target(X, Y);
}

void ps_cleanup() {
	GFraMe_tilemap_clear(&bg);
}

