/**
 * @src/score.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_tilemap.h>
#include "global.h"

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

void score_init() {
	int i = 0;
	// Assign basic tilemap data
	score_data[i++] = CHAR2TILE(' ');
	score_data[i++] = CHAR2TILE(' ');
	score_data[i++] = CHAR2TILE('S');
	score_data[i++] = CHAR2TILE('C');
	score_data[i++] = CHAR2TILE('O');
	score_data[i++] = CHAR2TILE('R');
	score_data[i++] = CHAR2TILE('E');
	score_data[i++] = CHAR2TILE(' ');
	score_data[i++] = CHAR2TILE(' ');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	score_data[i++] = CHAR2TILE('0');
	// Initialize the tilemap
	GFraMe_tilemap_init(&score, 9, 2, score_data, &gl_sset8, NULL, 0);
	score.x = 320 - 8*10;
	score.y = 8;
	// Resets the score
	cur_score = 0;
	i_val = 0;
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
}

void score_inc(int val) {
	double dif;
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
	return GFraMe_tilemap_draw(&score);
}

