/**
 * @src/score.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_save.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "global.h"

static GFraMe_save sav;
/**
 * Tilemap for the score
 */
static GFraMe_tilemap score;
/**
 * Data with things to be added
 */
static char score_data[9*2];
/**
 * How many actualy points the player has
 */
static int cur_score;
/**
 * Value being iterated (so it rolls to cur_score)
 */
static double i_val;
/**
 * How much should be added to i_val each second
 */
static double mod;

/**
 * Current highscore
 */
static int highscore = 0;
static int hs_time;

void score_init() {
	char *tmp;
	// Assign basic tilemap data
	tmp = GFraMe_str2tiles(score_data, "  SCORE  ", 0);
	tmp = GFraMe_str2tiles(tmp, "000000000", 0);
	// Initialize the tilemap
	GFraMe_tilemap_init(&score, 9, 2, score_data, &gl_sset8, NULL, 0);
	score.x = 320 - 8*10;
	score.y = 8;
	// Resets the score
	cur_score = 0;
	i_val = 0;
	hs_time = 0;
}

void highscore_init() {
	char *tmp;
	int i;
	
	// Set flashing time, if passed the score
	if (cur_score > highscore) {
		score_init();
		hs_time = 250;
	}
	else
		score_init();
	
	// Reassign tilemap data
	tmp = GFraMe_str2tiles(score_data, "HIGHSCORE", 0);
	
	GFraMe_save_bind(&sav, "bugsquasher.sav");
	if (GFraMe_save_read(&sav, "hs", &highscore, sizeof(int), 1) == GFraMe_ret_failed)
		highscore = 0;
	GFraMe_save_close(&sav);
	
	if (highscore > 0) {
		tmp += 8;
		i = highscore;
		while (i > 0) {
			// Update each digit on the tilemap
			*tmp-- = CHAR2TILE((i % 10) + '0');
			i /= 10;
		}
	}
}

void highscore_save() {
	if (cur_score > highscore) {
		if (cur_score > 999999999)
			cur_score = 999999999;
		GFraMe_save_bind(&sav, "bugsquasher.sav");
		GFraMe_save_write(&sav, "hs", &cur_score, sizeof(int), 1);
		GFraMe_save_close(&sav);
	}
}

void highscore_update(int ms) {
	if (hs_time > 0) {
		hs_time -= ms;
		if (hs_time <= 0)
			hs_time += 500;
	}
}

void score_update(int ms) {
	if (i_val < cur_score) {
		int tmp;
		int i;
		double time = ((double)ms) / 1000.0;
		// Update the displaying score
		i_val += time * mod;
		// Check if reached the current score
		if (i_val > cur_score)
			i_val = (double)cur_score;
		// Setup to looping
		tmp = (int)i_val;
		i = 17;
		// Updates the displayed score
		while (tmp > 0) {
			// Update each digit on the tilemap
			score_data[i--] = CHAR2TILE((tmp % 10) + '0');
			tmp /= 10;
		}
	}
	highscore_update(ms);
}

void score_inc(int val) {
	double dif;
	// Flash the score if it just passed the highscore
	if (cur_score <= highscore && cur_score + val > highscore)
		hs_time = 250;
	// Update the current score
	cur_score += val;
	// Calculate how fast it should increase per second
	dif = ((double)cur_score) - i_val;
	mod = dif / 2.5;
	// Shouldn't happen, but avoids problems
	if (mod <= 0.0)
		mod = 1.0;
}

GFraMe_ret score_draw() {
	if (hs_time < 250)
		return GFraMe_tilemap_draw(&score);
	else
		return GFraMe_ret_ok;
}

