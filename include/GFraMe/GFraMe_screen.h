/**
 * @include/GFraMe/GFraMe_screen.h
 */
#ifndef __GFRAME_SCREEN_H_
#define __GFRAME_SCREEN_H_

#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL_video.h>

enum enGFraMe_window_flags {
	GFraMe_window_maximized = SDL_WINDOW_MAXIMIZED,
	GFraMe_window_resizable = SDL_WINDOW_RESIZABLE,
	GFraMe_window_fullscreen = SDL_WINDOW_FULLSCREEN
};
typedef enum enGFraMe_window_flags GFraMe_window_flags;

/**
 * Window's width (read only)
 */
extern int GFraMe_window_w;
/**
 * Window's height (read only)
 */
extern int GFraMe_window_h;
/**
 * Virtual screen's width
 */
extern int GFraMe_screen_w;
/**
 * Virtual screen's height
 */
extern int GFraMe_screen_h;
/**
 * Backbuffer horizontal position on the screen
 */
extern int GFraMe_buffer_x;
/**
 * Backbuffer vertical position on the screen
 */
extern int GFraMe_buffer_y;
/**
 * Backbuffer actual rendered width
 */
extern int GFraMe_buffer_w;
/**
 * Backbuffer actual rendered height
 */
extern int GFraMe_buffer_h;
/**
 * Ratio between backbuffer's actual dimensions and
 *virtual screen dimensions
 */
extern double GFraMe_screen_ratio;

/**
 * Initialize SDL, already creating a window and a backbuffer.
 * @param	vw	Buffer's width (virtual width)
 * @param	vh	Buffer's height (virtual height)
 * @param	sw	Window's width (screen width); if 0, uses the device width
 * @param	sh	Window's height(screen height);if 0, uses the device height
 * @param	name	Window's title
 * @param	flags	Window creation flags
 * @param	fps		At how many frames per second the game should run;
 *				  notice that this is independent from update and render
 *				  rate, those should be set on each state
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_init(int vw, int vh, int sw, int sh, char *name,
				GFraMe_window_flags flags, int fps);

/**
 * Clean up memory allocated by init
 */
void GFraMe_quit();

/**
 * Get the dimensions of the first connected display
 * @param	*ret_width	Integer where the width is returned
 * @param	*ret_height	Integer where the height is returned
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_getDevDimensions(int *ret_width, int *ret_height);

/**
 * Converts a point from screen space to world space
 * @param	*x	Position with horizontal screen position and
 * 			  that will return the horizontal world position
 * @param	*y	Position with vertical screen position and
 * 			  that will return the vertical world position
 */
void GFraMe_screen_point_to_world(int *x, int *y);

/**
 * Try to set the device to a given width & height
 */
void GFraMe_set_screen_ratio();

/**
 * Set the color used to clear the screen
 * @param	red	Amount of red [0, 255]
 * @param	green	Amount of green [0, 255]
 * @param	blue	Amount of blue [0, 255]
 * @param	alpha	Amount of alpha [0, 255]; 0 - Translucent; 255 - Opaque
 */
void GFraMe_set_bg_color(char red, char green, char blue, char alpha);

/**
 * Must be called before drawing everything;
 * sets the backbuffer as the rendering target
 */
void GFraMe_init_render();

/**
 * Must be called after rendering everything;
 * actually renders the back buffer to the screen
 */
void GFraMe_finish_render();

#endif

