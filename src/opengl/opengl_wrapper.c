#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

GLW_RV glw_createCtx(SDL_Window *wnd) {
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	ctx = SDL_GL_CreateContext(wnd);
	if (!ctx)
		return GLW_FAILURE;
	
	glw_loadFunctions();
	
	return GLW_SUCCESS;
}

GLW_RV glw_compileProgram() {
	char *sprShd[2] = {sprVs, sprFs};
	char *bbShd[2] = {bbVs, bbFs};
	GLenum types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
	
	sprPrg = createProgram(types, sprShd, 2);
	if (sprPrg == 0)
		return GLW_FAILURE;
	
	bbPrg = createProgram(types, bbShd, 2);
	if (bbPrg == 0)
		return GLW_FAILURE;
	
	sprLocToGL = glGetUniformLocation(sprPrg, "locToGL");
	sprTexDimensions = glGetUniformLocation(sprPrg,"texDimensions");
	sprTranslation = glGetUniformLocation(sprPrg, "translation");
	sprDimension = glGetUniformLocation(sprPrg, "dimension");
	sprTexOffset = glGetUniformLocation(sprPrg, "texOffset");
	sprSampler = glGetUniformLocation(sprPrg, "gSampler");
	
	bbSampler = glGetUniformLocation(bbPrg, "gSampler");
	
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
	glUseProgram(0);
	
	return GLW_SUCCESS;
}

GLW_RV glw_createBackbuffer(int width, int height) {
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
	
	bbTex = 0;
	glGenTextures(1, &bbTex);
	if (bbTex == 0)
		return GLW_FAILURE;
	glBindTexture(GL_TEXTURE_2D, bbTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RGBA,
#if defined(RENDER_TO_BIG_BUFFER)
				width * HOR_SCALE,
				height * VER_SCALE,
#else
	             width,
	             height,
#endif
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
	
#if defined(RENDER_TO_BIG_BUFFER)
	worldMatrix[0] = 2.0f / (float)width;// * HOR_SCALE;
	worldMatrix[5] = -2.0f / (float)height;// * VER_SCALE;
#else
	worldMatrix[0] = 2.0f / (float)width;
	worldMatrix[5] = -2.0f / (float)height;
#endif
	
	glUseProgram(sprPrg);
	glUniformMatrix4fv(sprLocToGL, 1, GL_FALSE, worldMatrix);
	glUseProgram(0);
	
	return GLW_SUCCESS;
}

void glw_prepareRender() {
	glBindFramebuffer(GL_FRAMEBUFFER, bbFbo);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(sprPrg);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprTex);
	glUniform1i(sprSampler, 0);
	
	glBindVertexArray(sprVao);
}

void glw_renderSprite(int x, int y, int d, int tx, int ty) {
	glUniform2f(sprTranslation, (float)x, (float)y);
	glUniform1f(sprDimension, (float)d);
	glUniform2f(sprTexOffset, (float)tx, (float)ty);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void glw_doRender(SDL_Window *wnd) {
	glBindVertexArray(0);
	glUseProgram(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(bbPrg);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bbTex);
	glUniform1i(bbSampler, 0);
	glBindVertexArray(bbVao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	SDL_GL_SwapWindow(wnd);
}

void glw_cleanup() {
	if (bbTex)
		glDeleteTextures(1, &bbTex);
	if (bbFbo)
		glDeleteFramebuffers(1, &bbFbo);
	if (bbVao)
		glDeleteBuffers(1, &bbVao);
	if (bbIbo)
		glDeleteBuffers(1, &bbIbo);
	if (bbVbo)
		glDeleteBuffers(1, &bbVbo);
	if (sprTex)
		glDeleteTextures(1, &sprTex);
	if (sprVao)
		glDeleteBuffers(1, &sprVao);
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

