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
#define MAX_ENEMIES	32
/**
 * Array for every enemy (possibly) on the screen
 */
GFraMe_sprite enemies[MAX_ENEMIES];
/**
 * Background (image and for collision [when implemented])
 */
GFraMe_tilemap bg;
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

void ps_init() {
	int i;
	// Tilemap data for the background
	char bg_data[20*15] = {
0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3,
5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,
4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,
4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5
};
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
	i = 0;
	while (i < MAX_ENEMIES) {
		enemies[i].id = 0;
		i++;
	}
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
	GFraMe_event_update_begin();
		if (GFraMe_accumulator_loop(&acc_timer)) {
			int r;
			i = 0;
			while (i < MAX_ENEMIES) {
				if (!enemies[i].id) {
					enemies_spawn_random(enemies+i);
					break;
				}
				i++;
			}
			// Sets a new spawn time, depending on what was spawned
			if (i < MAX_ENEMIES) {
				// r = [-500, 500], with a step of 100
				r = (GFraMe_util_randomi() % 11 - 5) * 100;
				// easier enemies spawn faster! (also, 1s + r)
				r = (1000 + r) / (3 - (enemies[i].id - 1) % 3);
				GFraMe_accumulator_init_set(&acc_timer, r, r);
			}
		}
		// Check if the player is near the appex, and should slowdown
		if (player_slowdown())
			GFraMe_event_elapsed >>= 2;
		// Then, update the player
		player_update(GFraMe_event_elapsed);
		// Collide the player with the floor
		if (GFraMe_object_overlap(&ground, player_get_object(), GFraMe_first_fixed)
			== GFraMe_ret_ok)
			player_on_ground();
		// Update each enemy that has a type (set in 'id' field) and collide it with the player
		i = 0;
		while (i < MAX_ENEMIES) {
			if (enemies[i].id) {
				GFraMe_sprite *en = enemies + i;
				GFraMe_sprite_update(en, GFraMe_event_elapsed);
				if (en->obj.x > 320) {
					en->id = 0;
					i++;
					continue;
				}
				if (GFraMe_object_overlap(&en->obj, player_get_object(), GFraMe_dont_collide) == GFraMe_ret_ok) {
					//if (pl.obj.y < 144 &&
					//	(pl.obj.hit & GFraMe_direction_down)) {
						//pl.obj.vy = -200;
					if (player_on_squash() == GFraMe_ret_ok)
						en->id = 0;
						//tgt.id = 0;
					//}
					// TODO Do something on collision
				}
			}
			i++;
		}
	GFraMe_event_update_end();
}

void ps_do_draw() {
	int i;
	GFraMe_event_draw_begin();
		// Draw the tilemap
		GFraMe_tilemap_draw(&bg);
		// Draw each enemy that has a type (set in 'id' field)
		i = 0;
		while (i < MAX_ENEMIES) {
			if (enemies[i].id)
				GFraMe_sprite_draw(enemies + i);
			i++;
		}
		// Draw the player
		player_draw();
		//GFraMe_sprite_draw(&pl);
		// Draw the target, only if visible (lazily set as 'id' field)
		//if (tgt.id)
		//	GFraMe_sprite_draw(&tgt);
	GFraMe_event_draw_end();
}

void ps_on_click(int X, int Y) {
	if (player_jump(X) == GFraMe_ret_failed)
		player_set_target(X, Y);
}

void ps_cleanup() {
	GFraMe_tilemap_clear(&bg);
}

