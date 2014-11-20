/**
 * @src/gframe_screen.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_opengl.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_screen.h>
#include <SDL2/SDL.h>

/**
 * Application window. Not much else to say
 */
static SDL_Window *GFraMe_window = NULL;
//#if !defined(GFRAME_OPENGL)
/**
 * Renderer used to (duh) render stuff
 */
SDL_Renderer *GFraMe_renderer = NULL;
/**
 * Backbuffer where the game is rendered; is latter blitted into the window
 */
static SDL_Texture *GFraMe_screen;
//#endif
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
double GFraMe_screen_ratio_h = 0.0;
double GFraMe_screen_ratio_v = 0.0;

static Uint8 GFraMe_bg_r = 0xA0;
static Uint8 GFraMe_bg_g = 0xA0;
static Uint8 GFraMe_bg_b = 0xA0;
static Uint8 GFraMe_bg_a = 0xFF;

static void GFraMe_screen_cache_dimensions();
static void GFraMe_screen_log_dimensions(int zoom);
static void GFraMe_screen_log_format();
/**
 * Try to set the device to a given width & height
 */
static void GFraMe_set_screen_ratio();

/**
 * Initialize SDL, already creating a window and a backbuffer.
 * @param	vw	Buffer's width (virtual width)
 * @param	vh	Buffer's height (virtual height)
 * @param	sw	Window's width (screen width); if 0, uses the device width
 * @param	sh	Window's height(screen height);if 0, uses the device height
 * @param	name	Window's title
 * @param	flags	Window creation flags
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_screen_init(int vw, int vh, int sw, int sh, char *name,
				GFraMe_window_flags flags, GFraMe_wndext *ext) {
	GFraMe_ret rv = GFraMe_ret_ok;
	int w = 0, h = 0;
	// Get the device dimensions, in case it's needed
	rv = GFraMe_getDevDimensions(&w, &h);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to get device dimensions",
		_ret);
	// If no valid width was passed, use default one
	if (sw == 0)
		sw = w;
	// If no valid height was passed, use default one
	if (sh == 0)
		sh = h;
	
#if defined(GFRAME_OPENGL)
	GFraMe_opengl_setAtt();
	// Force OpenGL
	flags |= SDL_WINDOW_OPENGL;
#endif
	
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
	// Store backbuffer dimensions
	GFraMe_screen_w = vw;
	GFraMe_screen_h = vh;
#if defined(GFRAME_OPENGL)
	rv = GFraMe_opengl_init(ext->atlas, ext->atlasWidth, ext->atlasHeight,
			sw, sh, sw / vw, sh / vh, ext->flags);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to init opengl",
		rv = rv, _ret);
#else
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
#endif
	GFraMe_screen_log_format();
	// Set backbuffer dimensions and position
	GFraMe_set_screen_ratio();
_ret:
	return rv;
}

/**
 * Clean up memory allocated by init
 */
void GFraMe_screen_clean() {
#if defined(GFRAME_OPENGL)
	GFraMe_opengl_clear();
#else
	if (GFraMe_screen) {
		SDL_DestroyTexture(GFraMe_screen);
		GFraMe_screen = NULL;
	}
	if (GFraMe_renderer) {
		SDL_DestroyRenderer(GFraMe_renderer);
		GFraMe_renderer = NULL;
	}
#endif
	if (GFraMe_window) {
		SDL_DestroyWindow(GFraMe_window);
		GFraMe_window  = NULL;
	}
}

/**
 * Attach a 16x16 icon, of ARGB32 format, to the window.
 * @param	*pixels	Buffer of pixels in ARGB format
 */
GFraMe_ret GFraMe_set_icon(char *pixels) {
	GFraMe_ret rv;
	SDL_Surface *surf = NULL;
	Uint32 *pix32 = (Uint32*)pixels;
	
	GFraMe_assertRV(GFraMe_window, "Window not yet initialized",
		rv = GFraMe_ret_failed, _ret);
	
	// Check if byte order is correct and fix it
	if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
		int i = 0;
		int l = 16 * 16;
		while (i < l) {
			pix32[i] = SDL_Swap32(pix32[i]);
			i++;
		}
	}
	
	surf = SDL_CreateRGBSurfaceFrom(pix32, 16, 16, 32, 16*4, 0x00ff0000,
		0x0000ff00, 0x000000ff, 0xff000000);
	GFraMe_SDLassertRV(surf, "Failed to create surface",
		rv = GFraMe_ret_failed, _ret);
	
	SDL_SetWindowIcon(GFraMe_window, surf);
	
	rv = GFraMe_ret_ok;
_ret:
	return rv;
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
	*x = (int)((*x - GFraMe_buffer_x) / GFraMe_screen_ratio_h);
	*y = (int)((*y - GFraMe_buffer_y) / GFraMe_screen_ratio_v);
	return;
_exit:
	*x = -1;
	*y = -1;
}

/**
 * Store the dimensions on a SDL_rect, used during rendering
 */
static void GFraMe_screen_cache_dimensions() {
#if !defined(GFRAME_OPENGL)
	buffer_rect.x = GFraMe_buffer_x;
	buffer_rect.y = GFraMe_buffer_y;
	buffer_rect.w = GFraMe_buffer_w;
	buffer_rect.h = GFraMe_buffer_h;
#endif
}

/**
 * Set the destination dimensions keeping a perfect (i.e, integer) aspect ratio
 * @param	max_zoom	Maximum zoom value or a value equal to or less than 0
 *for any zoom
 * @param	update_window	Whether the windows dimensions should be updated
 * @return	The new zoom level or 0 on failure
 */
int GFraMe_screen_set_pixel_perfect(int max_zoom, int update_window) {
	int w = 0, h = 0, zoom, hratio, vratio;
	// Get the actual window's dimension
	if (update_window) {
		SDL_GetWindowSize(GFraMe_window, &w, &h);
		GFraMe_window_w = w;
		GFraMe_window_h = h;
	}
	else {
		w = GFraMe_window_w;
		h = GFraMe_window_h;
	}
	// Check each possible ratio
	hratio = (int)( (double)w / (double)GFraMe_screen_w );
	vratio = (int)( (double)h / (double)GFraMe_screen_h );
	// Get the lesser one
	if (hratio < vratio)
		zoom = hratio;
	else
		zoom = vratio;
	// Check if it's a valid zoom
	if (max_zoom > 0 && zoom > max_zoom)
		zoom = max_zoom;
	GFraMe_assertRet(zoom != 0, "Invalid aspect ratio", _ret);
	// Letterbox the game
	GFraMe_buffer_x = (w - GFraMe_screen_w * zoom) / 2;
	GFraMe_buffer_y = (h - GFraMe_screen_h * zoom) / 2;
	GFraMe_buffer_w = GFraMe_screen_w * zoom;
	GFraMe_buffer_h = GFraMe_screen_h * zoom;
	GFraMe_screen_ratio_h = zoom;
	GFraMe_screen_ratio_v = zoom;
	GFraMe_screen_log_dimensions(zoom);
	GFraMe_screen_cache_dimensions();
_ret:
	return zoom;
}

/**
 * Resize the game while keeping the original aspect ratio
 * @param	max_zoom	Maximum zoom value or a value equal to or less than 0
 *for any zoom
 * @param	update_window	Whether the windows dimensions should be updated
 * @return	The new zoom level or 0 on failure
 */
double GFraMe_screen_set_keep_ratio(int max_zoom, int update_window) {
	int w = 0, h = 0;
	double zoom, hratio, vratio;
	// Get the actual window's dimension
	if (update_window) {
		SDL_GetWindowSize(GFraMe_window, &w, &h);
		GFraMe_window_w = w;
		GFraMe_window_h = h;
	}
	else {
		w = GFraMe_window_w;
		h = GFraMe_window_h;
	}
	// Check each possible ratio
	hratio = (double)w / (double)GFraMe_screen_w;
	vratio = (double)h / (double)GFraMe_screen_h;
	if (vratio * GFraMe_screen_w <= w)
		zoom = vratio;
	else if (hratio * GFraMe_screen_h <= h)
		zoom = hratio;
	else
		zoom = 0;
	if (max_zoom > 0 && zoom > max_zoom)
		zoom = max_zoom;
	GFraMe_assertRet(zoom, "Invalid aspect ratio", _ret);
	// Letterbox the game
	GFraMe_buffer_x = (w - GFraMe_screen_w * zoom) / 2;
	GFraMe_buffer_y = (h - GFraMe_screen_h * zoom) / 2;
	GFraMe_buffer_w = GFraMe_screen_w * zoom;
	GFraMe_buffer_h = GFraMe_screen_h * zoom;
	GFraMe_screen_ratio_h = zoom;
	GFraMe_screen_ratio_v= zoom;
	GFraMe_screen_log_dimensions(zoom);
	GFraMe_screen_cache_dimensions();
_ret:
	return zoom;
}

/**
 * Resize the screen filling the greatest area possible, but ignoring the
 *original aspect ratio. This shouldn't be used at all! Ò.Ó
 * This version restrict the zoom the biggest integer possible.
 * @param	update_window	Whether the windows dimensions should be updated
 */
void GFraMe_screen_set_maximize_int(int update_window) {
	int w = 0, h = 0;
	// Get the actual window's dimension
	if (update_window) {
		SDL_GetWindowSize(GFraMe_window, &w, &h);
		GFraMe_window_w = w;
		GFraMe_window_h = h;
	}
	else {
		w = GFraMe_window_w;
		h = GFraMe_window_h;
	}
	// Get the current zoom level
	GFraMe_screen_ratio_h = (int)( (double)w / GFraMe_screen_w);
	GFraMe_screen_ratio_v = (int)( (double)h / GFraMe_screen_h);
	// Letterbox the game
	GFraMe_buffer_x = 0;
	GFraMe_buffer_y = 0;
	GFraMe_buffer_w = w;
	GFraMe_buffer_h = h;
	GFraMe_screen_log_dimensions(0);
	GFraMe_screen_cache_dimensions();
}

/**
 * Resize the screen filling the greatest area possible, but ignoring the
 *original aspect ratio. This shouldn't be used at all! Ò.Ó
 * @param	update_window	Whether the windows dimensions should be updated
 */
void GFraMe_screen_set_maximize_double(int update_window) {
	int w = 0, h = 0;
	// Get the actual window's dimension
	if (update_window) {
		SDL_GetWindowSize(GFraMe_window, &w, &h);
		GFraMe_window_w = w;
		GFraMe_window_h = h;
	}
	else {
		w = GFraMe_window_w;
		h = GFraMe_window_h;
	}
	// Letterbox the game
	GFraMe_buffer_x = 0;
	GFraMe_buffer_y = 0;
	GFraMe_buffer_w = w;
	GFraMe_buffer_h = h;
	GFraMe_screen_ratio_h = (double)w / GFraMe_screen_w;
	GFraMe_screen_ratio_v = (double)h / GFraMe_screen_h;
	GFraMe_screen_log_dimensions(0);
	GFraMe_screen_cache_dimensions();
}

/**
 * Try to set the device to a given width & height
 */
static void GFraMe_set_screen_ratio() {
	int i_zoom;
	double d_zoom;
	d_zoom = GFraMe_screen_set_keep_ratio(0, 1);
	i_zoom = (int)d_zoom;
	if ((double)i_zoom != d_zoom)
		d_zoom = (double)GFraMe_screen_set_pixel_perfect(0, 1);
	if (d_zoom == 0)
		GFraMe_screen_set_maximize_double(1);
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
#ifdef GFRAME_OPENGL
	GFraMe_opengl_prepareRender();
#else
	// Attach texture to the renderer
	SDL_SetRenderTarget(GFraMe_renderer, GFraMe_screen);
	// Set clear color
	SDL_SetRenderDrawColor(GFraMe_renderer, GFraMe_bg_r, GFraMe_bg_g,
						   GFraMe_bg_b, GFraMe_bg_a);
	// Clear the backbuffer
	SDL_RenderClear(GFraMe_renderer);
#endif
}

// TODO add post effects to screen (flash, shake, etc)

/**
 * Must be called after rendering everything;
 * actually renders the back buffer to the screen
 */
void GFraMe_finish_render() {
#ifdef GFRAME_OPENGL
	GFraMe_opengl_doRender();
#else
	// Detach the texture (attach it to the window)
	SDL_SetRenderTarget(GFraMe_renderer, NULL);
	// Set clear color
	SDL_SetRenderDrawColor(GFraMe_renderer, GFraMe_bg_r, GFraMe_bg_g,
						   GFraMe_bg_b, GFraMe_bg_a);
	// Clear the bg
	SDL_RenderClear(GFraMe_renderer);
	// Render the backbuffer
	SDL_RenderCopy(GFraMe_renderer, GFraMe_screen, NULL, &buffer_rect);
	// Switch buffers (blit to the screen)
	SDL_RenderPresent(GFraMe_renderer);
#endif
}

/**
 * Get the window reference
 */
SDL_Window *GFraMe_screen_get_window() {
	return GFraMe_window;
}

static void GFraMe_screen_log_dimensions(int zoom) {
	GFraMe_new_log("=============================");
	GFraMe_new_log("| Screen dimensions");
	GFraMe_new_log("-----------------------------");
	GFraMe_new_log("|   x: %i", GFraMe_buffer_x);
	GFraMe_new_log("|   y: %i", GFraMe_buffer_y);
	GFraMe_new_log("|   width: %i", GFraMe_buffer_w);
	GFraMe_new_log("|   height: %i", GFraMe_buffer_h);
	if (zoom != 0)
		GFraMe_new_log("|   multi: %i", zoom);
	GFraMe_new_log("=============================");
	GFraMe_new_log("");
}

#if !defined(GFRAME_OPENGL)
char* GFraMe_screen_print_pixelformat(Uint32 pfmt) {
	switch(pfmt) {
		case SDL_PIXELFORMAT_UNKNOWN: return "unknown";
		case SDL_PIXELFORMAT_INDEX1LSB: return "index 1 LSB";
		case SDL_PIXELFORMAT_INDEX1MSB: return "index 1 MSB";
		case SDL_PIXELFORMAT_INDEX4LSB: return "index 4 LSB";
		case SDL_PIXELFORMAT_INDEX4MSB: return "index 4 MSB";
		case SDL_PIXELFORMAT_INDEX8: return "index 8";
		case SDL_PIXELFORMAT_RGB332: return "RGB 332";
		case SDL_PIXELFORMAT_RGB444: return "RGB 444";
		case SDL_PIXELFORMAT_RGB555: return "RGB 555";
		case SDL_PIXELFORMAT_BGR555: return "BGR 555";
		case SDL_PIXELFORMAT_ARGB4444: return "ARGB4444";
		case SDL_PIXELFORMAT_RGBA4444: return "RGBA 4444";
		case SDL_PIXELFORMAT_ABGR4444: return "ABGR 4444";
		case SDL_PIXELFORMAT_BGRA4444: return "BGRA 4444";
		case SDL_PIXELFORMAT_ARGB1555: return "ARGB 1555";
		case SDL_PIXELFORMAT_RGBA5551: return "RGBA 5551";
		case SDL_PIXELFORMAT_ABGR1555: return "ABGR 1555";
		case SDL_PIXELFORMAT_BGRA5551: return "BGRA 5551";
		case SDL_PIXELFORMAT_RGB565: return "RGB 565";
		case SDL_PIXELFORMAT_BGR565: return "BGR 565";
		case SDL_PIXELFORMAT_RGB24: return "RGB 24";
		case SDL_PIXELFORMAT_BGR24: return "BGR 24";
		case SDL_PIXELFORMAT_RGB888: return "RGB 888";
		case SDL_PIXELFORMAT_RGBX8888: return "RGBX 8888";
		case SDL_PIXELFORMAT_BGR888: return "BGR 888";
		case SDL_PIXELFORMAT_BGRX8888: return "BGRX 8888";
		case SDL_PIXELFORMAT_ARGB8888: return "ARGB 8888";
		case SDL_PIXELFORMAT_RGBA8888: return "RGBA 8888";
		case SDL_PIXELFORMAT_ABGR8888: return "ABGR 8888";
		case SDL_PIXELFORMAT_BGRA8888: return "BGRA 8888";
		case SDL_PIXELFORMAT_ARGB2101010: return "ARGB 2 10 10 10";
		case SDL_PIXELFORMAT_YV12: return "YV12";
		case SDL_PIXELFORMAT_IYUV: return "IYUV";
		case SDL_PIXELFORMAT_YUY2: return "YUY2";
		case SDL_PIXELFORMAT_UYVY: return "UYVY";
		case SDL_PIXELFORMAT_YVYU: return "YVYU";
		default: return "unknown";
	}
}

static char* GFraMe_screen_print_access(int access) {
	switch (access) {
		case SDL_TEXTUREACCESS_STATIC: return "static";
		case SDL_TEXTUREACCESS_STREAMING: return "streaming";
		case SDL_TEXTUREACCESS_TARGET: return "target";
		default: return "unknown";
	}
}
#endif

static void GFraMe_screen_log_format() {
#if !defined(GFRAME_OPENGL)
	SDL_RendererInfo info;
	int i, access, w, h;
	Uint32 format;
	
	SDL_GetRendererInfo(GFraMe_renderer, &info);
	SDL_QueryTexture(GFraMe_screen, &format, &access, &w, &h);
	
	GFraMe_new_log("=============================");
	GFraMe_new_log(" | Renderer/Window info");
	GFraMe_new_log("-----------------------------");
	if (info.flags & SDL_RENDERER_SOFTWARE)
		GFraMe_new_log(" |   Supports software rendering");
	if (info.flags & SDL_RENDERER_ACCELERATED)
		GFraMe_new_log(" |   Supports hardware acceleration");
	if (info.flags & SDL_RENDERER_PRESENTVSYNC)
		GFraMe_new_log(" |   Supports vsync");
	if (info.flags & SDL_RENDERER_TARGETTEXTURE)
		GFraMe_new_log(" |   Supports rendering to texture");
	GFraMe_new_log("-----------------------------");
	GFraMe_new_log(" |   Pixel format: %s", GFraMe_screen_print_pixelformat(
		SDL_GetWindowPixelFormat(GFraMe_window)));
	GFraMe_new_log("-----------------------------");
	GFraMe_new_log(" |   Available texture formats:");
	i = 0;
	while (i < info.num_texture_formats) {
		GFraMe_new_log(" |     %s", GFraMe_screen_print_pixelformat(
			info.texture_formats[i]));
		i++;
	}
	GFraMe_new_log("-----------------------------");
	GFraMe_new_log(" |   Max texture width: %i", info.max_texture_width);
	GFraMe_new_log(" |   Max texture height: %i", info.max_texture_height);
	GFraMe_new_log("-----------------------------");
	GFraMe_new_log(" |   Backbuffer info:");
	GFraMe_new_log(" |     Pixel format: %s", GFraMe_screen_print_pixelformat(
		format));
	GFraMe_new_log(" |     Access mode: %s", GFraMe_screen_print_access(access));
	GFraMe_new_log(" |     Width: %i", w);
	GFraMe_new_log(" |     Height: %i", h);
	GFraMe_new_log("=============================");
	GFraMe_new_log("");
#endif
}

