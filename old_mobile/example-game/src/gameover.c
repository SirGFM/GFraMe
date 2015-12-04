/**
 * @src/gameover.c
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_pointer.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_tween.h>
#include "background.h"
#include "enemies.h"
#include "score.h"
#include "global.h"

GFraMe_event_setup();

enum {
	ENTER_G = 0,
	ENTER_A,
	ENTER_M,
	ENTER_E,
	ENTER_o,
	ENTER_v,
	ENTER_e,
	ENTER_r,
	WAIT_1,
	WAIT_2,
	WAIT_3,
	WAIT_4,
	EXIT_G,
	EXIT_A,
	EXIT_M,
	EXIT_E,
	EXIT_o,
	EXIT_v,
	EXIT_e,
	EXIT_r,
};

#define GMO_X	12
#define GMO_Y	29
#define VDIST	16
#define HDIST	37
enum {
	GMO_G = 0,
	GMO_A,
	GMO_M,
	GMO_E,
	GMO_o,
	GMO_v,
	GMO_e,
	GMO_r,
	MAX_SPRITES
};
static GFraMe_sprite gameover[MAX_SPRITES];

static GFraMe_accumulator timer;
static int state;
static int state_running;

static void gameover_init();
static void gameover_event();
static void gameover_update();
static void gameover_draw();
static void gameover_switch_state();

void gos_loop() {
	gameover_init();
	while (state_running && gl_running) {
		gameover_event();
		gameover_update();
		gameover_draw();
	}
}

static void gameover_init() {
	int i = 0;
	state_running = 1;
	state = ENTER_G;
	GFraMe_pointer_pressed = 0;
	while (i < MAX_SPRITES) {
		int X, Y, H, tile;
		GFraMe_sprite *spr = gameover + i;
		GFraMe_spriteset *sset;
		X = GMO_X + i * HDIST;
		Y = -64;
		if (i == GMO_G || i == GMO_o) {
			sset = &gl_sset32x64;
			H = 64;
		}
		else {
			sset = &gl_sset32;
			H = 32;
		}
		switch (i) {
			case GMO_G: tile = 8 * 3 + 2; break;
			case GMO_A: tile = 8 * 5 + 3; break;
			case GMO_M: tile = 8 * 6 + 4; break;
			case GMO_E:
			case GMO_e:
				tile = 8 * 5 + 6; break;
			case GMO_o: tile = 8 * 3 + 3; break;
			case GMO_v: tile = 8 * 6 + 5; break;
			case GMO_r: tile = 8 * 5 + 7; break;
			default: tile = 0;
		}
		GFraMe_sprite_init(spr, X, Y, 32, H, sset, 0, 0);
		spr->cur_tile = tile;
		i++;
	}
	// Init the timer
	GFraMe_accumulator_init_time(&timer, 100, 190);
	GFraMe_event_init(60, 60);
}

static void gameover_event() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
			GFraMe_accumulator_update(&timer, GFraMe_event_elapsed);
#ifdef GFRAME_MOBILE
		GFraMe_event_on_bg();
			GFraMe_audio_player_pause();
		GFraMe_event_on_fg();
			GFraMe_audio_player_play();
		GFraMe_event_on_finger_down();
#else
		GFraMe_event_on_mouse_down();
#endif
			gameover_switch_state();
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}

static void gameover_update() {
	int i;
	GFraMe_event_update_begin();
		i = 0;
		while (i < MAX_SPRITES) {
			GFraMe_sprite_update(gameover + i, GFraMe_event_elapsed);
			i++;
		}
		score_update(GFraMe_event_elapsed);
	GFraMe_event_update_end();
	if (GFraMe_accumulator_loop(&timer))
		gameover_switch_state();
}

static void gameover_draw() {
	int i;
	GFraMe_event_draw_begin();
		// Draw the background
		background_draw();
		// Draw every enemy
		enemies_draw();
		// Draw the current score
		score_draw();
		// Draw the letter
		i = 0;
		while (i < MAX_SPRITES) {
			GFraMe_sprite_draw(gameover + i);
			i++;
		}
	GFraMe_event_draw_end();
}

static void gameover_switch_state() {
	int X, fY, tY, i;
	GFraMe_tween *tw;
	switch (state) {
		case ENTER_G:
		case EXIT_G:
			i = GMO_G; break;
		case ENTER_A:
		case EXIT_A:
			i = GMO_A; break;
		case ENTER_M:
		case EXIT_M:
			i = GMO_M; break;
		case ENTER_E:
		case EXIT_E:
			i = GMO_E; break;
		case ENTER_o:
		case EXIT_o:
			i = GMO_o; break;
		case ENTER_v:
		case EXIT_v:
			i = GMO_v; break;
		case ENTER_e:
		case EXIT_e:
			i = GMO_e; break;
		case ENTER_r:
		case EXIT_r:
			i = GMO_r; break;
		default:
			state++;
			if (GFraMe_pointer_pressed || state >= EXIT_r)
				state_running = 0;
			return;
	}
	if (state < EXIT_G) {
		fY = -64;
		tY = GMO_Y;
		if (i != GMO_G && i != GMO_o)
			tY += VDIST;
	}
	else {
		fY = GMO_Y;
		if (i != GMO_G && i != GMO_o)
			fY += VDIST;
		tY = -64;
	}
	tw = GFraMe_sprite_get_tween(gameover + i);
	X = GMO_X + i * HDIST;
	GFraMe_tween_init(tw, X, fY, X, tY, 0.25, GFraMe_tween_lerp);
	state++;
}

