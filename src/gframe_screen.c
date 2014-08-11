/**
 * @src/gframe_screen.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_timer.h>
#include <SDL2/SDL.h>

/**
 * Application window. Not much else to say
 */
static SDL_Window *GFraMe_window = NULL;
/**
 * Renderer used to (duh) render stuff
 */
SDL_Renderer *GFraMe_renderer = NULL;
/**
 * Backbuffer where the game is rendered; is latter blitted into the window
 */
static SDL_Texture *GFraMe_screen;
/**
 * Timer used to issue new frames
 */
static GFraMe_timer timer = 0;
/**
 * Window region where the backbuffer is rendered
 */
static SDL_Rect buffer_rect;
/**
 * Window's width (read only)
 */
int GFraMe_window_w = 0;
/**
 * Window's height (read only)
 */
int GFraMe_window_h = 0;
/**
 * Virtual screen's width
 */
int GFraMe_screen_w = 0;
/**
 * Virtual screen's height
 */
int GFraMe_screen_h = 0;
/**
 * Backbuffer horizontal position on the screen
 */
int GFraMe_buffer_x = 0;
/**
 * Backbuffer vertical position on the screen
 */
int GFraMe_buffer_y = 0;
/**
 * Backbuffer actual rendered width
 */
int GFraMe_buffer_w = 0;
/**
 * Backbuffer actual rendered height
 */
int GFraMe_buffer_h = 0;
/**
 * Ratio between backbuffer's actual dimensions and
 *virtual screen dimensions
 */
double GFraMe_screen_ratio = 0.0;

static Uint8 GFraMe_bg_r = 0xA0;
static Uint8 GFraMe_bg_g = 0xA0;
static Uint8 GFraMe_bg_b = 0xA0;
static Uint8 GFraMe_bg_a = 0xFF;

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
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_init(int vw, int vh, int sw, int sh, char *name,
				GFraMe_window_flags flags, int fps) {
	GFraMe_ret rv = GFraMe_ret_ok;
	int w = 0, h = 0, ms = 0;
#ifdef DEBUG
	// Set logging, if debug
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif
	// Initialize SDL2
	rv = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	GFraMe_SDLassertRV(rv >= 0, "Couldn't initialize SDL", rv = GFraMe_ret_sdl_init_failed, _ret);
	// Get the device dimensions, in case it's needed
	rv = GFraMe_getDevDimensions(&w, &h);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to get device dimensions", _ret);
	// If no valid width was passed, use default one
	if (sw == 0)
		sw = w;
	// If no valid height was passed, use default one
	if (sh == 0)
		sh = h;
	// Create a window
	GFraMe_window = SDL_CreateWindow(name,
							         SDL_WINDOWPOS_UNDEFINED,
							         SDL_WINDOWPOS_UNDEFINED,
							         sw, sh,
							         flags);
	GFraMe_SDLassertRV(GFraMe_window, "Couldn't create a window",
					   rv = GFraMe_ret_window_creation_failed, _ret);
	// Store window dimensions
	GFraMe_window_w = sw;
	GFraMe_window_h = sh;
	// Create a renderer
	GFraMe_renderer = SDL_CreateRenderer(GFraMe_window, -1,
					SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	GFraMe_SDLassertRV(GFraMe_renderer, "Couldn't create renderer",
					   rv = GFraMe_ret_renderer_creation_failed, _ret);
	// Create a backbuffer
	GFraMe_screen = SDL_CreateTexture(GFraMe_renderer,
			SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, vw, vh);
	GFraMe_SDLassertRV(GFraMe_screen, "Couldn't create backbuffer",
					   rv = GFraMe_ret_backbuffer_creation_failed, _ret);
	// Store backbuffer dimensions
	GFraMe_screen_w = vw;
	GFraMe_screen_h = vh;
	// Set backbuffer dimensions and position
	GFraMe_set_screen_ratio();
	// Create a timer
	ms = GFraMe_timer_get_ms(fps);
	GFraMe_assertRV(ms > 0, "Requested FPS is too low", rv = GFraMe_ret_fps_req_low, _ret);
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
	if (GFraMe_screen) {
		SDL_DestroyTexture(GFraMe_screen);
		GFraMe_screen = NULL;
	}
	if (GFraMe_renderer) {
		SDL_DestroyRenderer(GFraMe_renderer);
		GFraMe_renderer = NULL;
	}
	if (GFraMe_window) {
		SDL_DestroyWindow(GFraMe_window);
		GFraMe_window  = NULL;
	}
	SDL_Quit();
}

/**
 * Get the dimensions of the first connected display
 * @param	*ret_width	Integer where the width is returned
 * @param	*ret_height	Integer where the height is returned
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_getDevDimensions(int *ret_width, int *ret_height) {
	GFraMe_ret rv = GFraMe_ret_ok;
	SDL_DisplayMode mode;
	
	// Check if there's at least one display connected
	GFraMe_assertRV(SDL_GetNumVideoDisplays() > 0,
					"Couldn't find any displays", rv = GFraMe_ret_num_display_failed, _ret);
	// Check if there's at least one display mode
	GFraMe_assertRV(SDL_GetNumDisplayModes(0) > 0,
					"Couldn't get any display modes", rv = GFraMe_ret_display_modes_failed, _ret);
	// Try to get it's dimensions
	rv = SDL_GetDisplayMode(0, 0, &mode);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to get the main display mode", _ret);
	// Return gotten dimensions
	*ret_width = mode.w;
	*ret_height = mode.h;
_ret:
	return rv;
}

/**
 * Converts a point from screen space to world space
 * @param	*x	Position with horizontal screen position and
 * 			  that will return the horizontal world position
 * @param	*y	Position with vertical screen position and
 * 			  that will return the vertical world position
 */
void GFraMe_screen_point_to_world(int *x, int *y) {
	GFraMe_assertRet(x != NULL, "Invalid 'x' parameter", _exit);
	GFraMe_assertRet(y != NULL, "Invalid 'y' parameter", _exit);
	*x = (int)((*x - GFraMe_buffer_x) / GFraMe_screen_ratio);
	*y = (int)((*y - GFraMe_buffer_y) / GFraMe_screen_ratio);
	return;
_exit:
	*x = -1;
	*y = -1;
}

/**
 * Try to set the device to a given width & height
 */
void GFraMe_set_screen_ratio() {
	// This was me being lazy, and I'm not sorry
	#define dmw ((double)GFraMe_window_w)
	#define dmh ((double)GFraMe_window_h)
	#define dw ((double)GFraMe_screen_w)
	#define dh ((double)GFraMe_screen_h)
	#define mw GFraMe_window_w
	#define mh GFraMe_window_h
	#define w GFraMe_screen_w
	#define h GFraMe_screen_h
	#define x GFraMe_buffer_x
	#define y GFraMe_buffer_y
	#define r GFraMe_screen_ratio
	// If vertical (device sideway) can be maximized
	if (dh / dmh * dw <= dmw)
		r = dmh / dh;
	// Else, try to maximize it on the horizontal
	else if (dw / dmw * dh <= dmh)
		r = dmw / dw;
	// If everything fail, do an actual letterboxing
	else {
		int hr = h / mh;
		int hw = w / mw;
		if (hr < hw)
			r = hr;
		else
			r = hw;
	}
	// Center the backbuffer
	x = (mw - w * r) / 2;
	y = (mh - h * r) / 2;
	// Set the backbuffer dimensions
	GFraMe_buffer_w = w * r;
	GFraMe_buffer_h = h * r;
	GFraMe_log("Final dimensions - x:%i y:%i width:%i height:%i multi:%.4f"
			   , x, y, GFraMe_buffer_w, GFraMe_buffer_h, r);
	#undef dmh
	#undef dmw
	#undef dh
	#undef dw
	#undef mh
	#undef mw
	#undef h
	#undef w
	#undef x
	#undef y
	buffer_rect.x = GFraMe_buffer_x;
	buffer_rect.y = GFraMe_buffer_y;
	buffer_rect.w = GFraMe_buffer_w;
	buffer_rect.h = GFraMe_buffer_h;
}

/**
 * Set the color used to clear the screen
 * @param	red	Amount of red [0, 255]
 * @param	green	Amount of green [0, 255]
 * @param	blue	Amount of blue [0, 255]
 * @param	alpha	Amount of alpha [0, 255]; 0 - Translucent; 255 - Opaque
 */
void GFraMe_set_bg_color(char red, char green, char blue, char alpha) {
	GFraMe_bg_r = red;
	GFraMe_bg_g = green;
	GFraMe_bg_b = blue;
	GFraMe_bg_a = alpha;
}

/**
 * Must be called before drawing everything;
 * sets the backbuffer as the rendering target
 */
void GFraMe_init_render() {
	// Attach texture to the renderer
	SDL_SetRenderTarget(GFraMe_renderer, GFraMe_screen);
	// Set clear color
	SDL_SetRenderDrawColor(GFraMe_renderer, GFraMe_bg_r, GFraMe_bg_g,
						   GFraMe_bg_b, GFraMe_bg_a);
	// Clear the backbuffer
	SDL_RenderClear(GFraMe_renderer);
}

// TODO add post effects to screen (flash, shake, etc)

/**
 * Must be called after rendering everything;
 * actually renders the back buffer to the screen
 */
void GFraMe_finish_render() {
	// Detach the texture (attach it to the window)
	SDL_SetRenderTarget(GFraMe_renderer, NULL);
	// Set clear color
	SDL_SetRenderDrawColor(GFraMe_renderer, GFraMe_bg_r, GFraMe_bg_g,
						   GFraMe_bg_b, GFraMe_bg_a);
	// Render the backbuffer
	SDL_RenderCopy(GFraMe_renderer, GFraMe_screen, NULL, &buffer_rect);
	// Switch buffers (blit to the screen)
	SDL_RenderPresent(GFraMe_renderer);
}

