/**
 * @src/playstate.c
 */
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "global.h"
#include "playstate.h"

// Define some variables needed by the events module
GFraMe_event_setup();

//====================================================//
// Local (to this state) variable (sprites and stuff) //
//====================================================//
/**
 * Player's sprite
 */
GFraMe_sprite pl;
/**
 * Target that the player will jump/dash toward
 */
GFraMe_sprite tgt;
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
4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
};
	// Initialize the player
	GFraMe_sprite_init(&pl, 10, 10, 8, 14, &gl_sset16, 4, 0);
	pl.cur_tile = 8;
	pl.obj.ay = 500;
	// Initialize the target
	GFraMe_sprite_init(&tgt, -16, -16, 16, 16, &gl_sset16, 0, 0);
	tgt.cur_tile = 9;
	tgt.id = 0;
	// Initialize the tilemap
	GFraMe_tilemap_init(&bg, 20, 15, bg_data, &gl_sset16, NULL, 0);
	// Create the floor for collisions
	GFraMe_object_clear(&ground);
	GFraMe_object_set_y(&ground, 160);
	GFraMe_object_set_hitbox(&ground, GFraMe_set_hitbox_upper_left,
							 0, 0, GFraMe_buffer_w, 16);
	// Initialize the timer and clean the events accumulated on the queue
	GFraMe_event_init(60, 60);
}

void ps_event_handler() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
		GFraMe_event_on_mouse_down();
			ps_on_click(GFraMe_event_mouse_x-8, GFraMe_event_mouse_y-8);
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}

void ps_do_update() {
	int pljump;
	GFraMe_event_update_begin();
		pljump = GFraMe_util_absd(pl.obj.vy) < 32.0;
		// Update the player's sprite
		if (pl.obj.hit & GFraMe_direction_down) {
			if (pl.obj.vy >= 0.0 && pl.obj.ax == 0.0f && pl.obj.vx != 0.0)
				pl.obj.ax = -pl.obj.vx * 4.0;
			else if(pl.obj.ax != 0.0&& GFraMe_util_absd(pl.obj.vx) <= 16.0){
				pl.obj.vx = 0.0;
				pl.obj.ax = 0.0;
			}
		}
		else if (pljump && tgt.id) {
			double X = tgt.obj.x - pl.obj.x;
			double Y = tgt.obj.y - pl.obj.y;
			double dist = GFraMe_util_sqrtd(X*X + Y*Y);
			pl.obj.vx = pl.obj.vx / 4.0 + X / dist * 200;
			pl.obj.vy = Y / dist * 200;
		}
		else if (pljump && !tgt.id) {
			GFraMe_event_elapsed >>= 2;
		}
		GFraMe_sprite_update(&pl, GFraMe_event_elapsed);
		// Collide the player with the floor
		if (GFraMe_object_overlap(&ground, &pl.obj, GFraMe_first_fixed)
			== GFraMe_ret_ok) {
			pl.obj.vy = 0.0;
			tgt.id = 0;
		}
	GFraMe_event_update_end();
}

void ps_do_draw() {
	GFraMe_event_draw_begin();
		GFraMe_tilemap_draw(&bg);
		GFraMe_sprite_draw(&pl);
		if (tgt.id)
			GFraMe_sprite_draw(&tgt);
	GFraMe_event_draw_end();
}

void ps_on_click(int X, int Y) {
	if (pl.obj.hit & GFraMe_direction_down) {
		pl.obj.vy = -200.0;
		pl.obj.vx = X - pl.obj.x;
		pl.obj.ax = 0.0;
	}
	else if (Y > 164 && GFraMe_util_absd(pl.obj.vy) < 64.0) {
		tgt.id = 1;
		GFraMe_object_set_pos(&tgt.obj, X, Y);
	}
}

void ps_cleanup() {
	GFraMe_tilemap_clear(&bg);
}

