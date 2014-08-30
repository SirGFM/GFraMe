/**
 * @src/playstate.c
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "background.h"
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
 * Initialize and run the play state loop
 */
void ps_loop() {
	ps_init();
	while (gl_running) {
		ps_event_handler();
		ps_do_update();
		ps_do_draw();
	}
	// TODO display stats
}

void ps_init() {
	// Initialize the player
	player_init();
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
		if (GFraMe_object_overlap(background_get_object(),
		                          player_get_object(),
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
		// Draw the background
		background_draw();
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

