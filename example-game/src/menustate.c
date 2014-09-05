/**
 * @src/menustate.h
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_messagebox.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include <stdlib.h>
#include "background.h"
#include "button.h"
#include "global.h"
#include "score.h"
#include "menustate.h"

#include <SDL2/SDL_video.h>

// Define some variables needed by the events module
GFraMe_event_setup();

enum {
	ENTER,
	GOTO_LOOP,
	LOOP,
	GOTO_EXIT,
	EXIT
};
static int state;

static int game_init;

static double time;
static GFraMe_accumulator timer;

static GFraMe_tilemap init_text;
static char init_data[20];
static int is_text_visible;

static Button gfm_bt;
static Button bt_1_1;
static Button bt_prop;
static Button bt_free;

#define BUG_X	110
#define BUG_Y	29
#define SQUASHER_X	16
#define SQUASHER_Y	65
#define VDIST	16
#define HDIST	37
enum {
	B=0,u,g, MAX_BUG
};
enum {
	S=0,q,u2,a,s,h,e,r,
	MAX_SQUASHER
};
static GFraMe_sprite BUG[MAX_BUG];
static GFraMe_sprite SQUASHER[MAX_SQUASHER];

static void menu_init();
static void menu_event();
static void menu_update();
static void menu_update_exit();
static void menu_draw();

static int can_switch;

void ms_loop() {
	menu_init();
	while (game_init == 0 && gl_running) {
		menu_event();
		switch (state) {
			case ENTER:
				time += (double)GFraMe_event_elapsed / 1000.0;
				if (time >= 1.5)
					state = GOTO_LOOP;
				else {
					menu_update();
					is_text_visible = 0;
				}
			break;
			case GOTO_LOOP:
				#define SPR_POS(TYPE, LETTER) \
					GFraMe_object_set_pos(&TYPE[LETTER].obj, \
										  TYPE##_X + HDIST * LETTER, \
										  TYPE##_Y + VDIST*(LETTER != 0));\
					TYPE[LETTER].obj.vy = 0
				SPR_POS(BUG, B);
				SPR_POS(BUG, u);
				SPR_POS(BUG, g);
				SPR_POS(SQUASHER, S);
				SPR_POS(SQUASHER, q);
				SPR_POS(SQUASHER, u2);
				SPR_POS(SQUASHER, a);
				SPR_POS(SQUASHER, s);
				SPR_POS(SQUASHER, h);
				SPR_POS(SQUASHER, e);
				SPR_POS(SQUASHER, r);
				#undef SPR_POS
				state = LOOP;
			break;
			case LOOP: menu_update(); break;
			case GOTO_EXIT: {
				#define SPR_ACC(TYPE, LETTER) \
					TYPE[LETTER].obj.ay = -8 * TYPE[LETTER].obj.hitbox.hh - 2*TYPE[LETTER].obj.y
				SPR_ACC(BUG, B);
				SPR_ACC(BUG, u);
				SPR_ACC(BUG, g);
				SPR_ACC(SQUASHER, S);
				SPR_ACC(SQUASHER, q);
				SPR_ACC(SQUASHER, u2);
				SPR_ACC(SQUASHER, a);
				SPR_ACC(SQUASHER, s);
				SPR_ACC(SQUASHER, h);
				SPR_ACC(SQUASHER, e);
				SPR_ACC(SQUASHER, r);
				#undef SPR_ACC
				GFraMe_accumulator_init_time(&timer, 250, 400);
				time = 0.0;
				state = EXIT;
				GFraMe_audio_play(&gl_start, 0.75);
			} break;
			case EXIT: {
				menu_update_exit();
				time += (double)GFraMe_event_elapsed / 1000.0;
				if (time > 1.5)
					game_init = 1;
			};
		}
		menu_draw();
	}
}

static void menu_init() {
	int i;
	i = 0;
	game_init = 0;
	time = 0;
	state = ENTER;
	can_switch = 0;
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
	// Initialize the title
	#define INIT_SPR(TYPE, LETTER, H, SSET, TILE) \
		GFraMe_sprite_init(TYPE + LETTER, TYPE##_X + HDIST*LETTER, \
						   -H, 32, H, \
						   &gl_sset##SSET, 0, 0); \
		TYPE[LETTER].cur_tile = TILE; \
		TYPE[LETTER].obj.vy = H + TYPE##_Y + VDIST*(LETTER != 0); \
		if (TYPE == SQUASHER) { \
			GFraMe_object_set_y(&TYPE[LETTER].obj, \
								(int)(TYPE[LETTER].obj.y \
									  - TYPE[LETTER].obj.vy)); \
			TYPE[LETTER].obj.vy *= 2; \
		}
	INIT_SPR(BUG, B, 64, 32x64, 8*3);
	INIT_SPR(BUG, u, 32, 32, 8*5);
	INIT_SPR(BUG, g, 32, 32, 8*5+1);
	INIT_SPR(SQUASHER, S, 64, 32x64, 8*3+1);
	INIT_SPR(SQUASHER, u2, 32, 32, 8*5);
	INIT_SPR(SQUASHER, q, 32, 32, 8*5+2);
	INIT_SPR(SQUASHER, a, 32, 32, 8*5+3);
	INIT_SPR(SQUASHER, s, 32, 32, 8*5+4);
	INIT_SPR(SQUASHER, h, 32, 32, 8*5+5);
	INIT_SPR(SQUASHER, e, 32, 32, 8*5+6);
	INIT_SPR(SQUASHER, r, 32, 32, 8*5+7);
	#undef INIT_SPR
	// Initialize the timer and clean the events accumulated on the queue
	GFraMe_accumulator_init_time(&timer, 500, 900);
	GFraMe_event_init(60, 60);
}

static void menu_event() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
		GFraMe_event_on_mouse_moved();
		GFraMe_event_on_mouse_down();
			switch (state) {
				case ENTER: state = GOTO_LOOP; break;
				case LOOP: if (can_switch) state = GOTO_EXIT; break;
				case EXIT: game_init = 1; break;
			}
		GFraMe_event_on_mouse_up();
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}

static void menu_update() {
	GFraMe_event_update_begin();
		int i;
		GFraMe_accumulator_update(&timer, GFraMe_event_elapsed);
		if (GFraMe_accumulator_loop(&timer)) {
			is_text_visible = !is_text_visible;
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
			if (  gfm_bt.state == RELEASED
					&& bt_1_1.state == RELEASED
					&& bt_prop.state == RELEASED
					&& bt_free.state == RELEASED)
				can_switch = 1;
			else
				can_switch = 0;
		}
		i = 0;
		while (i < MAX_BUG) {
			GFraMe_sprite_update(BUG + i, GFraMe_event_elapsed);
			i++;
		}
		i = 0;
		while (i < MAX_SQUASHER) {
			GFraMe_sprite_update(SQUASHER + i, GFraMe_event_elapsed);
			i++;
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
		while (i < MAX_BUG) {
			GFraMe_sprite_draw(BUG + i);
			i++;
		}
		i = 0;
		while (i < MAX_SQUASHER) {
			GFraMe_sprite_draw(SQUASHER + i);
			i++;
		}
	GFraMe_event_draw_end();
}

static void menu_update_exit() {
	GFraMe_event_update_begin();
		int i;
		GFraMe_accumulator_update(&timer, GFraMe_event_elapsed);
		if (GFraMe_accumulator_loop(&timer)) {
			is_text_visible = !is_text_visible;
		}
		i = 0;
		while (i < MAX_BUG) {
			GFraMe_sprite_update(BUG + i, GFraMe_event_elapsed);
			i++;
		}
		i = 0;
		while (i < MAX_SQUASHER) {
			GFraMe_sprite_update(SQUASHER + i, GFraMe_event_elapsed);
			i++;
		}
	GFraMe_event_update_end();
}

