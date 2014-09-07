/**
 * @src/menustate.c
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_messagebox.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_tween.h>
#include <GFraMe/GFraMe_util.h>
#include <stdlib.h>
#include "background.h"
#include "button.h"
#include "global.h"
#include "score.h"
#include "menustate.h"

// Define some variables needed by the events module
GFraMe_event_setup();

enum {
	ENTER,
	LOOP,
	EXIT
};
static int state;

static int game_init;

static GFraMe_accumulator timer;

static GFraMe_tilemap init_text;
static char init_data[20];
static int is_text_visible;
static int requestSwitch;

static Button gfm_bt;
static Button bt_1_1;
static Button bt_prop;
static Button bt_free;

#define BUG_X	110
#define BUG_Y	29
#define SQS_X	16
#define SQS_Y	65
#define VDIST	16
#define HDIST	37
enum {
	BUG_B,
	BUG_u,
	BUG_g,
	SQS_S,
	SQS_q,
	SQS_u,
	SQS_a,
	SQS_s,
	SQS_h,
	SQS_e,
	SQS_r,
	MAX_SPRITES
};
static GFraMe_sprite title[MAX_SPRITES];

static void menu_init();
static void menu_event();
static void menu_update();
static void menu_draw();
static void menu_switch_state();

void ms_loop() {
	menu_init();
	while (game_init == 0 && gl_running) {
		requestSwitch = 0;
		menu_event();
		menu_update();
		menu_draw();
		menu_switch_state();
	}
}

static void menu_init() {
	int i;
	int len;
	char tmp[20];
	char *ptC;
	game_init = 0;
	state = ENTER;
	requestSwitch = 0;
	// Initialize the background
	background_init();
	// Initialize the buttons
	button_init(&gfm_bt, 0, 240-44, 60, 61, 61, -1);
	button_init(&bt_1_1, 320-44*3, 240-44, 8, 9, 10, 38);
	button_init(&bt_prop, 320-44*2, 240-44, 8, 9, 10, 39);
	button_init(&bt_free, 320-44, 240-44, 8, 9, 10, 54);
	// Initialize the highscore
	highscore_init();
	// Set init text
	is_text_visible = 1;
	GFraMe_tilemap_init(&init_text, 20, 1, init_data, &gl_sset8, NULL, 0);
	init_text.x = (320 - 20*8) / 2;
	init_text.y = 240 - 32 - 16 - 24;
	len = 20;
	ptC = GFraMe_util_strcat(tmp, "-- ", &len);
#ifdef MOBILE
	ptC = GFraMe_util_strcat(ptC, "TOUCH", &len);
#else
	ptC = GFraMe_util_strcat(ptC, "CLICK", &len);
#endif
	ptC = GFraMe_util_strcat(ptC, " TO START --", &len);
	GFraMe_str2tiles(init_data, tmp, 0);
	// Init every sprite
	i = 0;
	while (i < MAX_SPRITES) {
		GFraMe_sprite *spr = title + i;
		GFraMe_object *obj = GFraMe_sprite_get_object(spr);
		GFraMe_tween *tw = GFraMe_sprite_get_tween(spr);
		if (i == BUG_B) {
			GFraMe_sprite_init(spr, BUG_X, BUG_Y, 32, 64,&gl_sset32x64,0,0);
			spr->cur_tile = 8*3;
		}
		else if (i == SQS_S) {
			GFraMe_sprite_init(spr, SQS_X, SQS_Y, 32, 64,&gl_sset32x64,0,0);
			spr->cur_tile = 8*3+1;
		}
		else if (i < SQS_S) {
			GFraMe_sprite_init(spr, BUG_X,BUG_Y+VDIST,32,32,&gl_sset32,0,0);
			spr->cur_tile = 8*5 + i - BUG_u;
		}
		else {
			GFraMe_sprite_init(spr, SQS_X,SQS_Y+VDIST,32,32,&gl_sset32,0,0);
			if (i == SQS_u)
				spr->cur_tile = 8*5;
			else
				spr->cur_tile = 8*5 + i - SQS_S;
		}
		if (i == SQS_q)
			spr->cur_tile++;
		// Set horizontal position and tween
		if (i < SQS_S) {
			GFraMe_object_set_x(obj, obj->x + i * HDIST);
			GFraMe_tween_init(tw, obj->x, -64, obj->x, obj->y, 1.5,
							  GFraMe_tween_lerp);
		}
		else {
			GFraMe_object_set_x(obj, obj->x + (i - SQS_S) * HDIST);
			GFraMe_tween_init(tw, obj->x, -64, obj->x, obj->y, 0.75,
							  GFraMe_tween_lerp);
		}
		i++;
	}
	// Init the timer
	GFraMe_accumulator_init_time(&timer, 500, 900);
	GFraMe_event_init(60, 60);
}

static void menu_event() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
		GFraMe_event_on_mouse_moved();
		GFraMe_event_on_finger_down();
			requestSwitch = 1;
		GFraMe_event_on_finger_up();
		GFraMe_event_on_mouse_down();
			requestSwitch = 1;
		GFraMe_event_on_mouse_up();
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}

static void menu_update() {
	int i;
	GFraMe_event_update_begin();
		GFraMe_accumulator_update(&timer, GFraMe_event_elapsed);
		if (GFraMe_accumulator_loop(&timer)) {
			is_text_visible = !is_text_visible;
		}
		i = 0;
		while (i < MAX_SPRITES) {
			GFraMe_sprite_update(title + i, GFraMe_event_elapsed);
			i++;
		}
		// Update the buttons
		if (state == LOOP) {
			highscore_update(GFraMe_event_elapsed);
			button_update(&gfm_bt, GFraMe_event_elapsed, GFraMe_event_mouse_x,
						  GFraMe_event_mouse_y, GFraMe_event_mouse_pressed);
			button_update(&bt_1_1, GFraMe_event_elapsed, GFraMe_event_mouse_x,
						  GFraMe_event_mouse_y, GFraMe_event_mouse_pressed);
			button_update(&bt_prop, GFraMe_event_elapsed, GFraMe_event_mouse_x,
						  GFraMe_event_mouse_y, GFraMe_event_mouse_pressed);
			button_update(&bt_free, GFraMe_event_elapsed, GFraMe_event_mouse_x,
						  GFraMe_event_mouse_y, GFraMe_event_mouse_pressed);
			if (gfm_bt.justReleased) {
				GFraMe_button_ret res;
				res = GFraMe_messagebox_OkCancel("Open author's website?",
									"Go to http://gfmgamecorner.wordpres.com?",
									"Open", "Cancel");
				if (res == GFraMe_button_ret_ok)
					GFraMe_util_open_browser("http://gfmgamecorner.wordpress.com/");
			}
			else if (bt_1_1.justReleased)
				GFraMe_screen_set_pixel_perfect(0, 1);
			else if (bt_prop.justReleased) 
				GFraMe_screen_set_keep_ratio(0, 1);
			else if (bt_free.justReleased)
				GFraMe_screen_set_maximize_double(1);
		}
	GFraMe_event_update_end();
}

static void menu_draw() {
	GFraMe_event_draw_begin();
		int i;
		// Draw the background
		background_draw();
		// Draw the buttons
		if (state == LOOP) {
			button_draw(&gfm_bt);
			button_draw(&bt_1_1);
			button_draw(&bt_prop);
			button_draw(&bt_free);
			score_draw();
		}
		// Draw start message
		if (is_text_visible)
			GFraMe_tilemap_draw(&init_text);
		i = 0;
		while (i < MAX_SPRITES) {
			GFraMe_sprite_draw(title + i);
			i++;
		}
	GFraMe_event_draw_end();
}

static void menu_switch_state() {
	if (state == LOOP) {
		if (   gfm_bt.state != RELEASED
			|| bt_1_1.state != RELEASED
			|| bt_prop.state != RELEASED
			|| bt_free.state != RELEASED
			|| !requestSwitch)
		return;
	}
	else if (requestSwitch)
		{}
	else if (!GFraMe_tween_is_complete(GFraMe_sprite_get_tween(title)))
		return;
	
	if (state == ENTER) {
		int i;
		i = 0;
		while (i < MAX_SPRITES) {
			GFraMe_object *obj = GFraMe_sprite_get_object(title + i);
			GFraMe_tween *tw = GFraMe_sprite_get_tween(title + i);
			GFraMe_object_set_y(obj, tw->toY);
			GFraMe_tween_clear(tw);
			i++;
		}
	}
	else if (state == LOOP) {
		int i;
		i = 0;
		while (i < MAX_SPRITES) {
			double time;
			GFraMe_object *obj = GFraMe_sprite_get_object(title + i);
			GFraMe_tween *tw = GFraMe_sprite_get_tween(title + i);
			if (i < SQS_S)
				time = 1.5;
			else
				time = 0.75;
			GFraMe_tween_init(tw, obj->x, obj->y, obj->x, -64, time,
							  GFraMe_tween_lerp);
			i++;
		}
	}
	else
		game_init = 1;
	state++;
}

