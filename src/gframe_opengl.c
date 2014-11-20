/**
 * @src/gframe_opengl.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_screen.h>
#include <stdlib.h>
#include "opengl/opengl_wrapper.h"

extern SDL_Window *GFraMe_screen_get_window();

#define ASSERT(rv) \
	do { \
		if (rv != GLW_SUCCESS) \
			goto __ret; \
	} while(0)

GFraMe_ret GFraMe_opengl_init(char *texF, int texW, int texH, int winW,
	int winH, int sX, int sY, GFraMe_wndext_flags flags) {
	GLW_RV rv;
	GFraMe_ret grv;
	char *data = NULL;
	
	grv = GFraMe_assets_buffer_image(texF, texW, texH, &data);
	GFraMe_assertRV(grv == GFraMe_ret_ok, "Failed to read file",
		rv = GLW_FAILURE, __ret);

	rv = glw_createCtx(GFraMe_screen_get_window());
	ASSERT(rv);
	
	rv = glw_compileProgram(flags & GFraMe_wndext_scanline);
	ASSERT(rv);
	
	rv = glw_createSprite(texW, texH, data);
	ASSERT(rv);
	
	rv = glw_createBackbuffer(winW / sX, winH / sY, sX, sY);
	ASSERT(rv);
	
__ret:
	if (data)
		free(data);
	if (rv == GLW_SUCCESS)
		return GFraMe_ret_ok;
	return GFraMe_ret_failed;
}

void GFraMe_opengl_clear() {
	glw_cleanup();
}

void GFraMe_opengl_setAtt() {
	glw_setAttr();
}

void GFraMe_opengl_prepareRender() {
	glw_prepareRender();
}

void GFraMe_opengl_renderSprite(int x, int y, int dx, int dy, int tx, int ty) {
	glw_renderSprite(x, y, dx, dy, tx, ty);
}

void GFraMe_opengl_doRender() {
	glw_doRender(GFraMe_screen_get_window());
}

