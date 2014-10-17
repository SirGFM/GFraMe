/**
 * @src/gframe.c
 */
#include <GFraMe/GFraMe.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_timer.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL.h>

/**
 * "Organization" name. Is used as part of paths.
 */
char GFraMe_org[GFraMe_max_org_len];
/**
 * Game's title. Is used as part of paths.
 */
char GFraMe_title[GFraMe_max_game_title_len];
/**
 * Path to the directory where the game is running
 */
char GFraMe_path[GFraMe_max_path_len];

/**
 * Timer used to issue new frames
 */
static GFraMe_timer timer = 0;

/**
 * Initialize SDL, already creating a window and a backbuffer.
 * @param	vw	Buffer's width (virtual width)
 * @param	vh	Buffer's height (virtual height)
 * @param	sw	Window's width (screen width); if 0, uses the device width
 * @param	sh	Window's height(screen height);if 0, uses the device height
 * @param	org	Organization's name (used by the log and save file)
 * @param	name	Game's name (also used as window's title)
 * @param	flags	Window creation flags
 * @param	fps		At how many frames per second the game should run;
 *				  notice that this is independent from update and render
 *				  rate, those should be set on each state
 * @param	log_to_file	Whether should log to a file or to the terminal
 * @param	log_append	Whether should overwrite or append to an existing log
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_init(int vw, int vh, int sw, int sh, char *org,
	char *name, GFraMe_window_flags flags, int fps,
	int log_to_file, int log_append) {
	
	GFraMe_ret rv = GFraMe_ret_ok;
	int ms = 0, len;
	
	// Store organization name and game's title so it can be used for
	//logging and saving
	len = GFraMe_max_org_len;
	GFraMe_util_strcat(GFraMe_org, org, &len);
	len = GFraMe_max_game_title_len;
	GFraMe_util_strcat(GFraMe_title, name, &len);
	
#ifndef GFRAME_MOBILE
	// Also, get current directory
	char *tmp = SDL_GetBasePath();
	GFraMe_SDLassertRV(tmp, "Couldn't get current running path",
		rv = GFraMe_ret_failed, _ret);
	len = GFraMe_max_path_len;
	GFraMe_util_strcat(GFraMe_path, tmp, &len);
	SDL_free(tmp);
#endif
	
	if (log_to_file)
		GFraMe_log_init(log_append);
#ifdef GFRAME_DEBUG
	// Set logging, if debug
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif
	
	// Initialize SDL2
	rv = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	GFraMe_SDLassertRV(rv >= 0, "Couldn't initialize SDL",
		rv = GFraMe_ret_sdl_init_failed, _ret);
	
	// Initialize the screen
	rv = GFraMe_screen_init(vw, vh, sw, sh, name, flags);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to initialize the screen",
		rv=rv, _ret);
	
	// Create a timer
	ms = GFraMe_timer_get_ms(fps);
	GFraMe_assertRV(ms > 0, "Requested FPS is too low",
		rv = GFraMe_ret_fps_req_low, _ret);
	rv = GFraMe_timer_init(ms, &timer);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to create timer", _ret);
_ret:
	return rv;
}

/**
 * Clean up memory allocated by init
 */
void GFraMe_quit() {
	if (timer) {
		GFraMe_timer_stop(timer);
		timer = 0;
	}
	GFraMe_screen_clean();
	GFraMe_log_close();
	SDL_Quit();
}

