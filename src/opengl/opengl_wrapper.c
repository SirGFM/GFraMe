#include <SDL2/SDL.h>
#if defined(GFRAME_MOBILE)
#  include <SDL2/SDL_opengles2.h>
#else
#  include <SDL2/SDL_opengl.h>
#endif
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_log.h>
#include <stdio.h>
#include <stdlib.h>
#include "opengl_wrapper.h"
// import a few functions implementations (and variable declarations),
//to keep this source clean
#include "glw_functions.h"
#include "glw_static.h"
#include "glw_shaders.h"

void glw_setAttr() {
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if defined(GFRAME_MOBILE)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
}

GLW_RV glw_createCtx(SDL_Window *wnd) {
	GLint vp[4];
	
	ctx = SDL_GL_CreateContext(wnd);
	if (!ctx)
		return GLW_FAILURE;
	
	glw_loadFunctions();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glGetIntegerv(GL_VIEWPORT, vp);
	
	GFraMe_new_log(
"=============================\n"
" |  OpenGL viewport:\n"
"-----------------------------\n"
" |    x: %i\n"
" |    y: %i\n"
" |    w: %i\n"
" |    h: %i\n"
"=============================\n\n", vp[0], vp[1], vp[2], vp[3]);
	
	return GLW_SUCCESS;
}

GLW_RV glw_compileProgram(int use_scanlines) {
	char *sprShd[2] = {sprVs, sprFs};
	char *bbShd[2] = {bbVs, bbFs};
	GLenum types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
	
	if (!use_scanlines)
		bbShd[1] = bbFs_noSL;
	
	sprPrg = createProgram(types, sprShd, 2);
	if (sprPrg == 0)
		return GLW_FAILURE;
	
	bbPrg = createProgram(types, bbShd, 2);
	if (bbPrg == 0)
		return GLW_FAILURE;
	
	sprLocToGL = glGetUniformLocation(sprPrg, "locToGL");
	sprTexDimensions = glGetUniformLocation(sprPrg,"texDimensions");
	sprTranslation = glGetUniformLocation(sprPrg, "translation");
	sprRotation = glGetUniformLocation(sprPrg, "rotation");
	sprScale = glGetUniformLocation(sprPrg, "scale");
	sprDimensions = glGetUniformLocation(sprPrg, "dimensions");
	sprTexOffset = glGetUniformLocation(sprPrg, "texOffset");
	sprSampler = glGetUniformLocation(sprPrg, "gSampler");
	sprAlpha = glGetUniformLocation(sprPrg, "alpha");
	
	bbSampler = glGetUniformLocation(bbPrg, "gSampler");
	bbTexDimensions = glGetUniformLocation(bbPrg, "texDimensions");
	
	return GLW_SUCCESS;
}

GLW_RV glw_createSprite(int width, int height, char *data) {
	float vbo_data[] = {-0.5f,-0.5f, -0.5f,0.5f, 0.5f,0.5f, 0.5f,-0.5f};
	GLshort ibo_data[] = {0,1,2, 2,3,0};
	
	sprVbo = 0;
	glGenBuffers(1, &sprVbo);
	if (sprVbo == 0)
		return GLW_FAILURE;
	glBindBuffer(GL_ARRAY_BUFFER, sprVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	sprIbo = 0;
	glGenBuffers(1, &sprIbo);
	if (sprIbo == 0)
		return GLW_FAILURE;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_data), ibo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
#if !defined(GFRAME_MOBILE)
	sprVao = 0;
	glGenVertexArrays(1, &sprVao);
	if (sprVao == 0)
		return GLW_FAILURE;
	glBindVertexArray(sprVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sprVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
#endif
	
	sprTex = 0;
	glGenTextures(1, &sprTex);
	if (sprTex == 0)
		return GLW_FAILURE;
	glBindTexture(GL_TEXTURE_2D, sprTex);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RGBA,
	             width,
	             height,
	             0,
	             GL_RGBA,
	             GL_UNSIGNED_BYTE,
	             data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glUseProgram(sprPrg);
	glUniform2f(sprTexDimensions, 1.0f / (float)width, 1.0f / (float)height);
	glUniform2f(sprRotation, 1.0f, 0.0f);
	glUniform2f(sprScale, 1.0f, 1.0f);
	glUniform1f(sprAlpha, 1.0f);
	glUseProgram(0);
	
	return GLW_SUCCESS;
}

GLW_RV glw_createBackbuffer(int width, int height, int sX, int sY) {
	float vbo_data[] = {-1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f, 1.0f,-1.0f};
	GLshort ibo_data[] = {0,1,2, 2,3,0};
	GLenum status;
	
	bbVbo = 0;
	glGenBuffers(1, &bbVbo);
	if (bbVbo == 0)
		return GLW_FAILURE;
	glBindBuffer(GL_ARRAY_BUFFER, bbVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	bbIbo = 0;
	glGenBuffers(1, &bbIbo);
	if (bbIbo == 0)
		return GLW_FAILURE;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_data), ibo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
#if !defined(GFRAME_MOBILE)
	bbVao = 0;
	glGenVertexArrays(1, &bbVao);
	if (bbVao == 0)
		return GLW_FAILURE;
	glBindVertexArray(bbVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, bbVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
#endif
	
	bbTex = 0;
	glGenTextures(1, &bbTex);
	if (bbTex == 0)
		return GLW_FAILURE;
	glBindTexture(GL_TEXTURE_2D, bbTex);
#if !defined(GFRAME_MOBILE)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// TODO make the texture smaller than the destination window
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RGBA,
	             width,
	             height,
	             0,
	             GL_RGBA,
	             GL_UNSIGNED_BYTE,
	             NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	bbFbo = 0;
	glGenFramebuffers(1, &bbFbo);
	if (bbFbo == 0)
		return GLW_FAILURE;
	glBindFramebuffer(GL_FRAMEBUFFER, bbFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
	                       GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D,
	                       bbTex,
	                       0);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		return GLW_FAILURE;
	
	worldMatrix[0] = 2.0f / (float)width;
	worldMatrix[5] = -2.0f / (float)height;
	
	glUseProgram(sprPrg);
	glUniformMatrix4fv(sprLocToGL, 1, GL_FALSE, worldMatrix);
	glUseProgram(bbPrg);
	glUniform2f(bbTexDimensions, 1.0f / (float)width, 1.0f / (float)height);
	glUseProgram(0);
	
	return GLW_SUCCESS;
}

void glw_prepareRender() {
	glBindFramebuffer(GL_FRAMEBUFFER, bbFbo);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(sprPrg);
	glViewport(0, 0, GFraMe_screen_w, GFraMe_screen_h);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprTex);
	glUniform1i(sprSampler, 0);
	
#if !defined(GFRAME_MOBILE)
	glBindVertexArray(sprVao);
#else
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sprVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
#endif
}

void glw_setRotation(float angle) {
	glUniform2f(sprRotation, 1.0f, 0.0f);
}

void glw_setScale(float sX, float sY) {
	glUniform2f(sprScale, sX, sY);
}

void glw_setAlpha(float alpha) {
	glUniform1f(sprAlpha, alpha);
}

void glw_renderSprite(int x, int y, int dx, int dy, int tx, int ty) {
	glUniform2f(sprTranslation, (float)x, (float)y);
	glUniform2f(sprDimensions, (float)dx, (float)dy);
	glUniform2f(sprTexOffset, (float)tx, (float)ty);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void glw_doRender(SDL_Window *wnd) {
#if !defined(GFRAME_MOBILE)
	glBindVertexArray(0);
#endif
	glUseProgram(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(bbPrg);
	//glViewport(0, 0, GFraMe_window_w, GFraMe_window_h);
	glViewport(GFraMe_buffer_x,
	           GFraMe_buffer_y,
	           GFraMe_buffer_w,
			   GFraMe_buffer_h);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bbTex);
	glUniform1i(bbSampler, 0);
#if !defined(GFRAME_MOBILE)
	glBindVertexArray(bbVao);
#else
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, bbVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
#endif
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
#if !defined(GFRAME_MOBILE)
	glBindVertexArray(0);
#endif
	glUseProgram(0);
	
	SDL_GL_SwapWindow(wnd);
}

void glw_cleanup() {
	if (bbTex)
		glDeleteTextures(1, &bbTex);
	if (bbFbo)
		glDeleteFramebuffers(1, &bbFbo);
#if !defined(GFRAME_MOBILE)
	if (bbVao)
		glDeleteBuffers(1, &bbVao);
#endif
	if (bbIbo)
		glDeleteBuffers(1, &bbIbo);
	if (bbVbo)
		glDeleteBuffers(1, &bbVbo);
	if (sprTex)
		glDeleteTextures(1, &sprTex);
#if !defined(GFRAME_MOBILE)
	if (sprVao)
		glDeleteBuffers(1, &sprVao);
#endif
	if (sprIbo)
		glDeleteBuffers(1, &sprIbo);
	if (sprVbo)
		glDeleteBuffers(1, &sprVbo);
	if (bbPrg)
		glDeleteProgram(bbPrg);
	if (sprPrg)
		glDeleteProgram(sprPrg);
	if (ctx)
		SDL_GL_DeleteContext(ctx);
}

