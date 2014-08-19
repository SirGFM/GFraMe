/**
 * @src/gframe_texture.c
 */
#include <GFraMe/GFraMe_texture.h>
#include <SDL2/SDL.h>

/**
 * From @src/gframe_screen.c;
 * target for rendering and needed to create textures
 */
extern SDL_Renderer *GFraMe_renderer;

/**
 * Initialize a texture (so every field is actually 'nil')
 */
void GFraMe_texture_init(GFraMe_texture *tex) {
	// Set the texture to NULL (important, in case an error happens
	// and there's an generic cleanup code
	tex->texture = NULL;
	// Those aren't as important, though...
	tex->w = -1;
	tex->h = -1;
	tex->is_target = 0;
}

/**
 * Clear up a texture
 */
void GFraMe_texture_clear(GFraMe_texture *tex) {
	// Destroy an existing texture...
	if (tex->texture)
		SDL_DestroyTexture(tex->texture);
	// And clear the references
	GFraMe_texture_init(tex);
}


/**
 * Creates a texture that can be blitted into
 * @param	*out	GFraMe_texture created (allocated by caller!)
 * @param	width	Texture's width
 * @param	height	Texture's height
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_create_blank(GFraMe_texture *out,
								int width, int height) {
	GFraMe_ret rv = GFraMe_ret_ok;
	SDL_Texture *tex = NULL;
	// Try to create a texture that can be drawn onto
	tex = SDL_CreateTexture(GFraMe_renderer, SDL_PIXELFORMAT_ARGB8888,
							SDL_TEXTUREACCESS_TARGET, width, height);
	GFraMe_SDLassertRV(tex, "Couldn't create texture", rv = GFraMe_ret_texture_creation_failed, _ret);
	// Create a GFraMe_texture for returning
	out->texture = tex;
	out->w = width;
	out->h = height;
	out->is_target = 1;
_ret:
	return rv;
}

/**
 * Loads a texture's data into a renderable texture
 * @param	*out	GFraMe_texture created (allocated by caller!)
 * @param	width	Texture's width
 * @param	height	Texture's height
 * @param	*data	Input data (must actually be formatted RGBA)
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_load(GFraMe_texture *out, int width, int height,
						unsigned char *data) {
	GFraMe_ret rv = GFraMe_ret_ok;
	SDL_Texture *tex = NULL;
	// Create a texture
	tex = SDL_CreateTexture(GFraMe_renderer, SDL_PIXELFORMAT_ARGB8888,
							SDL_TEXTUREACCESS_STATIC, width, height);
	GFraMe_SDLassertRV(tex, "Couldn't create texture", rv = GFraMe_ret_texture_creation_failed, _ret);
	// Upload data to the texture
	rv = SDL_UpdateTexture(tex, NULL, (const void*)data,
						width*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888));
	GFraMe_SDLassertRet(rv == 0, "Failed to upload data to texture", _ret);
	// Make it translucent (where alpha == 0 [I think])
	rv = SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	GFraMe_SDLassertRet(rv == 0, "Failed to set blend mode", _ret);
	// Create output texture
	out->texture = tex;
	out->w = width;
	out->h = height;
	out->is_target = 0;
	// Clear up SDL texture
	tex = NULL;
_ret:
	if (tex)
		SDL_DestroyTexture(tex);
	return rv;
}

/**
 * Used by lock, unlock and copy to store the previous target
 */
static SDL_Texture *prev_target = NULL;

/**
 * Set some internal state to use l_copy
 * @param *tex	Texture that will be drawn into
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_lock(GFraMe_texture *tex) {
	GFraMe_ret rv = GFraMe_ret_ok;
	// Check if param is ok
	GFraMe_assertRV(tex, "Bad parameter!", rv = GFraMe_ret_bad_param, _ret);
	// Check if texture is target
	GFraMe_assertRV(tex->is_target, "Texture can't be targeted!",
					rv = GFraMe_ret_invalid_texture, _ret);
	// Store the previous target
	prev_target = SDL_GetRenderTarget(GFraMe_renderer);
	// Set this as the new target
	SDL_SetRenderTarget(GFraMe_renderer, tex->texture);
_ret:
	return rv;
}

/**
 * Return state so everything renders correctly
 */
void GFraMe_texture_unlock() {
	SDL_SetRenderTarget(GFraMe_renderer, prev_target);
}

/**
 * Copy into the texture defined at lock (no check is made!!)
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*tex	Source texture for copying
 */
GFraMe_ret GFraMe_texture_l_copy(int sx, int sy, int sw, int sh,
						  int dx, int dy, int dw, int dh,
						  GFraMe_texture *tex) {
	int rv = 0;
	SDL_Rect src;
	SDL_Rect dst;
	// Set up src info
	src.x = sx;
	src.y = sy;
	src.w = sw;
	src.h = sh;
	// Set up dst info
	dst.x = dx;
	dst.y = dy;
	dst.w = dw;
	dst.h = dh;
	// Copy it
	rv = SDL_RenderCopy(GFraMe_renderer, tex->texture, &src, &dst);
	GFraMe_SDLassertRet(rv == 0, "Failed to copy", _ret);
_ret:
	return rv;
}

/**
 * Copy into the texture defined at lock (no check is made!!),
 * flipping source horizontally
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*tex	Source texture for copying
 */
GFraMe_ret GFraMe_texture_l_copy_flipped(int sx, int sy, int sw, int sh,
						  int dx, int dy, int dw, int dh,
						  GFraMe_texture *tex) {
	int rv = 0;
	SDL_Rect src;
	SDL_Rect dst;
	// Set up src info
	src.x = sx;
	src.y = sy;
	src.w = sw;
	src.h = sh;
	// Set up dst info
	dst.x = dx;
	dst.y = dy;
	dst.w = dw;
	dst.h = dh;
	// Copy it
	rv = SDL_RenderCopyEx(GFraMe_renderer, tex->texture, &src, &dst, 0.0, NULL, SDL_FLIP_HORIZONTAL);
	GFraMe_SDLassertRet(rv == 0, "Failed to copy", _ret);
_ret:
	return rv;
}

/**
 * Copies portion of a texture into another texture
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*src	Source texture for copying
 * @param	*dst	Target texture
 */
GFraMe_ret GFraMe_texture_copy(int sx, int sy, int sw, int sh,
						int dx, int dy, int dw, int dh,
						GFraMe_texture *src, GFraMe_texture *dst) {
	GFraMe_ret rv = GFraMe_ret_ok;
	// Lock the texture
	rv = GFraMe_texture_lock(dst);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to lock texture!", _bad_param);
	// Copy it
	rv = GFraMe_texture_l_copy(sx, sy, sw, sh, dx, dy, dw, dh, src);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to copy!", _ret);
_ret:
	GFraMe_texture_unlock();
_bad_param:
	return rv;
}

