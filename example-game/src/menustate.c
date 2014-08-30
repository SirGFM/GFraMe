/**
 * @src/menustate.h
 */
#include <GFraMe/GFraMe_accumulator.h>
#include <GFraMe/GFraMe_event.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "background.h"
#include "global.h"
#include "score.h"
#include "menustate.h"

// Define some variables needed by the events module
GFraMe_event_setup();

static int game_init;

static GFraMe_tilemap init_text;
static char init_data[20];
static void menu_init();
static void menu_event();
static void menu_update();
static void menu_draw();

void ms_loop() {
	menu_init();
	while (game_init == 0 && gl_running) {
		menu_event();
		menu_update();
		menu_draw();
	}
}

static void menu_init() {
	int i;
	i = 0;
	game_init = 0;
	// Initialize the background
	background_init();
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
	// Initialize the timer and clean the events accumulated on the queue
	GFraMe_event_init(60, 60);
}

static void menu_event() {
	GFraMe_event_begin();
		GFraMe_event_on_timer();
		GFraMe_event_on_mouse_down();
			game_init = 1;
		GFraMe_event_on_quit();
			GFraMe_log("Received quit!");
			gl_running = 0;
	GFraMe_event_end();
}
static void menu_update() {
	GFraMe_event_update_begin();
	GFraMe_event_update_end();
}
static void menu_draw() {
	GFraMe_event_draw_begin();
		// Draw the background
		background_draw();
		// Draw start message
		GFraMe_tilemap_draw(&init_text);
	GFraMe_event_draw_end();
}

