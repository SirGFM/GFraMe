/**
 * @src/score.c
 */
#include <string.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_save.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <GFraMe/GFraMe_util.h>
#include "global.h"

#ifdef GFRAME_MOBILE
static GFraMe_save_ret old_save_goto_ID_position(GFraMe_save *sv, char *id);
static GFraMe_ret old_save_read_id(GFraMe_save *sv, char *id);
static GFraMe_ret old_save_read(GFraMe_save *sv, char *id,
		void *data, int size, int count);
#endif

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
	GFraMe_ret rv;
	
	// Set flashing time, if passed the score
	if (cur_score > highscore) {
		score_init();
		hs_time = 250;
	}
	else
		score_init();
	
	// Reassign tilemap data
	tmp = GFraMe_str2tiles(score_data, "HIGHSCORE", 0);
	
	rv = GFraMe_save_bind(&sav, "bugsquasher.sav");
#if 0
#  ifdef GFRAME_MOBILE
	if (rv == GFraMe_no_version_found) {
		int iTmp;
		rv = old_save_read(&sav, "hs", &iTmp, sizeof(int), 1);
		GFraMe_save_erase(&sav);
		
		GFraMe_save_write_int(&sav, "GFraMe_V", 0x00010000);
		GFraMe_save_close(&sav);
		
		GFraMe_save_bind(&sav, "bugsquasher.sav");
		if (rv != GFraMe_ret_failed)
			GFraMe_save_write_int(&sav, "highscore", iTmp);
		GFraMe_save_close(&sav);
		
		GFraMe_save_bind(&sav, "bugsquasher.sav");
	}
#  endif
#endif
	//if (GFraMe_save_read(&sav, "hs", &highscore, sizeof(int), 1) == GFraMe_ret_failed)
	if (GFraMe_save_read_int(&sav, "highscore", &highscore) != GFraMe_ret_ok)
		highscore = 0;
	GFraMe_save_close(&sav);
	memset(&sav, 0x0, sizeof(GFraMe_save));
	
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
	#ifdef GFRAME_MOBILE
		GFraMe_save_erase(&sav);
		GFraMe_save_close(&sav);
		GFraMe_save_bind(&sav, "bugsquasher.sav");
	#endif
		//GFraMe_save_write(&sav, "hs", &cur_score, sizeof(int), 1);
		GFraMe_save_write_int(&sav, "highscore", cur_score);
		GFraMe_save_close(&sav);
		memset(&sav, 0x0, sizeof(GFraMe_save));
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
	if (cur_score > 999999999)
		cur_score = 999999999;
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

#if 0
#  ifdef GFRAME_MOBILE
GFraMe_ret old_save_read(GFraMe_save *sv, char *id,
		void *data, int size, int count) {
	GFraMe_ret rv = GFraMe_ret_ok;
	GFraMe_save_ret srv = GFraMe_save_ret_ok;
	srv = old_save_goto_ID_position(sv, id);
	if (srv == GFraMe_save_ret_ok) {
		char tmp;
		SDL_RWread(sv->file, &tmp, sizeof(char), 1);
		if (tmp == size*count) {
			GFraMe_new_log("YAY!!");
			SDL_RWread(sv->file, data, size, count);
			rv = GFraMe_ret_ok;
		}
		else
			rv = GFraMe_ret_failed;
	}
	else
		rv = GFraMe_ret_failed;
	return rv;
}

static GFraMe_save_ret old_save_goto_ID_position(GFraMe_save *sv, char *id) {
	int pos;
	GFraMe_save_ret rv = GFraMe_save_ret_ok;
	char buf[GFraMe_save_max_len];
	// Check if the file isn't empty
	GFraMe_assertRV(sv->size > 0, "File's empty",
					rv = GFraMe_save_ret_empty, _ret);
	// Go back to the file's begin
	pos = SDL_RWseek(sv->file, 0, SEEK_SET);
	GFraMe_assertRV(pos >= 0, "ERROR", rv = GFraMe_save_ret_failed, _ret);
	while (1) {
		int res;
		char len;
		GFraMe_assertRV(pos != sv->size, "ID not found",
						rv = GFraMe_save_ret_id_not_found, _ret);
		// Try to read the current id
		rv = old_save_read_id(sv, buf);
		GFraMe_assertRet(rv == GFraMe_save_ret_ok, "Failed to seek id", _ret);
		// Exit loop if it was found
		if (GFraMe_util_strcmp(id, buf) == GFraMe_ret_ok) {
			GFraMe_new_log("Found a match!");
			break;
		}
		// Skip the data
		res = SDL_RWread(sv->file, &len, sizeof(char), 1);
		GFraMe_assertRV(res > 0, "ERROR", rv = GFraMe_save_ret_failed, _ret);
		pos = SDL_RWseek(sv->file, len, SEEK_CUR);
	}
	rv = GFraMe_save_ret_ok;
_ret:
	return rv;
}

static GFraMe_ret old_save_read_id(GFraMe_save *sv, char *id) {
	int rv;
	char id_len;
	// Try to read the id length
	rv = SDL_RWread(sv->file, &id_len, sizeof(char), 1);
	GFraMe_SDLassertRet(rv == 1, "Couldn't read id len", _ret);
	// Try to read the id
	rv = SDL_RWread(sv->file, id, sizeof(char), id_len);
	GFraMe_SDLassertRet(rv == id_len, "Failed to read id", _ret);
	// id_len is at most 255, so no check is needed
	id[(int)id_len] = '\0';
	if (rv != 0)
		GFraMe_new_log("Found id %s", id);
_ret:
	return (rv != 0)?GFraMe_ret_ok:GFraMe_ret_failed;
}
#  endif
#endif

