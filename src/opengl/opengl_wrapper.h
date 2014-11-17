/**
 * @file opengl_wrapper.h
 * 
 * Simple wrapper for OpenGL built over SDL2. It works for single window
 *applications
 * 
 * @author GFM
 */
#ifndef __OPENGL_WRAPPER_H_
#define __OPENGL_WRAPPER_H_

#include <SDL2/SDL.h>

typedef enum {
	GLW_SUCCESS = 0,
	GLW_FAILURE
} GLW_RV;

/**
 * Set a few attributes, as bits per color
 */
void glw_setAttr();

/**
 * Create a OpenGL context for the given window, as well as load
 */
GLW_RV glw_createCtx(SDL_Window *wnd);

/**
 * Compile both the sprite and backbuffer programs, as well as set its uniforms
 */
GLW_RV glw_compileProgram();

/**
 * Create all the needed buffers to render a sprite
 */
GLW_RV glw_createSprite(int width, int height, char *data);

/**
 * Create all the needed buffers (and texture) to create a backbuffer
 */
GLW_RV glw_createBackbuffer(int width, int height);

/**
 * Setup the state to render sprites to the backbuffer
 */
void glw_prepareRender();

/**
 * Render one sprite to the backbuffer
 */
void glw_renderSprite(int x, int y, int d, int tx, int ty);

/**
 * Render the backbuffer to the screen
 */
void glw_doRender();

/**
 * Cleanup memory used by this wrapper
 */
void glw_cleanup();

#endif

