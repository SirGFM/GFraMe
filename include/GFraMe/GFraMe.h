/**
 * @include/GFraMe/GFraMe.h
 */
#ifndef __GFRAME_H_
#define __GFRAME_H_

#include <GFraMe/GFraMe_screen.h>

#define GFraMe_major_version	0
#define GFraMe_minor_version	1
#define GFraMe_fix_version		0
#define GFraMe_version	"0.1.0"

#define GFraMe_max_org_len	80
/**
 * "Organization" name. Is used as part of paths.
 */
extern char GFraMe_org[GFraMe_max_org_len];

#define GFraMe_max_game_title_len 80
/**
 * Game's title. Is used as part of paths.
 */
extern char GFraMe_title[GFraMe_max_game_title_len];

#define GFraMe_max_path_len	512
/**
 * Path to the directory where the game is running
 */
extern char GFraMe_path[GFraMe_max_path_len];

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
				int log_to_file, int log_append);

void GFraMe_quit();

#endif

