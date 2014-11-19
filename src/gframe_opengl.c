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

#if 0

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_GLContext GFraMe_opengl_ctx;
static GLuint GFraMe_spriteVbo;
static GLuint GFraMe_spriteIbo;
static GLuint GFraMe_spriteTex;
static GLuint GFraMe_spriteVao;

static GLuint GFraMe_program;

static GLuint GFraMe_locToGL;
static GLuint GFraMe_texDimensions;
static GLuint GFraMe_translation;
static GLuint GFraMe_dimension;
static GLuint GFraMe_texOffset;
static GLuint GFraMe_sampler;

static GLuint GFraMe_ppProgram;

static GLuint GFraMe_ppSampler;
static GLuint GFraMe_ppWindowDimensions;
static GLuint GFraMe_ppTexDimensions;

static GLuint GFraMe_bbTexture;
static GLuint GFraMe_bbFbo;
static GLuint GFraMe_bbVbo;
static GLuint GFraMe_bbVao;

static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
#if defined(_WIN32) || defined(_WIN64)
static PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
static PFNGLBINDSAMPLERPROC glBindSampler;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLUNIFORM1IPROC glUniform1i;
static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLDETACHSHADERPROC glDetachShader;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram;

extern SDL_Window *GFraMe_screen_get_window();

static float GFraMe_worldMatrix[16] = 
	{1.0f, 0.0f, 0.0f,-1.0f,
	 0.0f, 1.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f};

// Shaders for rendering the sprites to the backbuffer
static const char GFraMe_vertexShader[] = 
  "#version 330\n"
  "layout(location = 0) in vec2 vtx;\n" // since it's a simple sprite, both
                                        //tile and position are mapped by the
                                        //same plane

  "uniform mat4 locToGL;\n"             // world to screen transform
  "uniform vec2 texDimensions;\n"       // width and height of the texture, but
                                        //as (1/width, 1/height)
  "uniform vec2 translation;\n"         // offset from (0,0) (world space)
  "uniform float dimension;\n"           // sprite dimension (e.g., 16 -> 16x16)
  "uniform vec2 texOffset;\n"           // tile position on the texture
  
  "out vec2 texCoord0;\n"
  
  "void main() {\n"
  
  "  vec2 texCoord;\n"
  "  vec2 pos;\n"
  
  "  pos  = vtx;\n"                     // 
  "  pos *= dimension;\n"               //
  "  pos += dimension*vec2(0.5f,"       //
  "                            0.5f);\n"// set the origin at the top left corner
  "  pos += translation;\n"             // 
  "  vec4 position = vec4(pos.x, pos.y,"// convert it to a vec4
  "                     -1.0f, 1.0f);\n"
  "  gl_Position = position*locToGL;\n" // output vertex to OpenGL
  
  "  texCoord = vtx + vec2(0.5f,"       // now it's a square's vertex of side 1
                             "0.5f);\n" //and origin (0,0)
  "  texCoord *= texDimensions;\n"      // make it 1-pixel wide
  "  texCoord *= dimension;\n"          // make it "1-tile" wide
  "  texCoord0 = texDimensions*texOffset"
  "                +texCoord;\n"        // select which tile to use
  "}\n";

static const char GFraMe_fragmentShader[] = 
	"#version 330\n"
	"in vec2 texCoord0;\n"
	
	"uniform sampler2D gSampler;\n"
	
	"void main()\n"
	"{\n"
	"  gl_FragColor = texture2D(gSampler, texCoord0.st);\n"
	//"  gl_FragColor.rb = outputColor.br;\n"
	//"	gl_FragColor = vec4(1.0f,0.0f,0.0f,1.0f);\n"
	"}\n";

// Shaders for rendering the backbuffer to the screen (Post Process)
static const char GFraMe_vertexPPShader[] = 
	"#version 330\n"
	"layout(location = 0) in vec2 pos;\n"
	"out vec2 _texCoord;\n"
	"out vec2 tmp;\n"
	
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(pos, -1.0f, 1.0f);\n"
	"	_texCoord = 0.5f * pos + vec2(0.5f, 0.5f);\n"
	"	_texCoord.y *= -1.0f;\n"
	"	tmp = pos + vec2(2.0, 4.0f);\n"
	"}\n";

static const char GFraMe_fragmentPPShader[] = 
	"#version 330\n"
	"in vec2 _texCoord;\n"
	"in vec2 tmp;\n"
	
	"uniform sampler2D gPpSampler;\n"
	"uniform vec2 windowDimensions;\n"
	"uniform vec2 ppTexDimensions;\n"
//	"uniform float offsetY;\n"
//	"uniform vec4 flashColor;\n"
//	"uniform lowp float flashT;\n"
//	"uniform lowp float fadeT;\n"
	
	"void main()\n"
	"{\n"
	"	vec2 texCoord = _texCoord;\n"
	
		// Shake effect
//	"  texCoord.t += offsetY;\n"
	"  lowp vec4 pixel = texture2D(gPpSampler, texCoord.st);\n"
		// Flash effect
//	"  pixel = pixel*(1-flashT) + flashT*flashColor;\n"
		// Fade effect
//	"  pixel = pixel*(1-fadeT) + fadeT*vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
	
	"  gl_FragColor = pixel;\n"
	//"  glFragColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);\n"
	//"  glFragColor = vec4(tmp/8.0f, 0.0f, 1.0f);\n"
	"}\n";

static void GFraMe_opengl_createSpriteBuffers();

static GFraMe_ret GFraMe_opengl_createSpriteTexture(char *file, int width,
	int height);

static GFraMe_ret GFraMe_opengl_createShaders();

static void GFraMe_opengl_createVertexArrayObject();

static GFraMe_ret GFraMe_opengl_createBackbuffer(int w, int h, int sX, int sY);

static void GFraMe_opengl_resetScreen(float x, float y, float w, float h,
	float sX, float sY);

static GLuint GFraMe_opengl_createProgram(GLenum shaderTypes[],
	char *shaders[], int num);

static GLuint GFraMe_opengl_compileShader(GLenum eShaderType,
	char *strShaderFile);

static GFraMe_ret GFraMe_opengl_loadFunctions();

static void GFraMe_opengl_validateProgram(GLuint program);

GFraMe_ret GFraMe_opengl_init(char *texF, int texW, int texH, int winW,
	int winH, int sX, int sY) {
	GFraMe_ret rv;
	
	GFraMe_opengl_ctx = SDL_GL_CreateContext(GFraMe_screen_get_window());
	GFraMe_assertRV(GFraMe_opengl_ctx, "OpenGL ctx ERR", rv = GFraMe_ret_failed,
		__ret);
	
	rv = GFraMe_opengl_loadFunctions();
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	GFraMe_opengl_createSpriteBuffers();
	
	GFraMe_opengl_createVertexArrayObject();
	
	rv = GFraMe_opengl_createShaders();
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	glUseProgram(GFraMe_program);
	glUniform2f(GFraMe_texDimensions, 1.0f / texW, 1.0f / texH);
	glUniform1i(GFraMe_sampler, 0);
	
	glUseProgram(GFraMe_ppProgram);
	glUniform2f(GFraMe_ppTexDimensions, texW, texH);
	glUniform1i(GFraMe_ppSampler, 0);
	
	glUseProgram(0);
	
	rv = GFraMe_opengl_createBackbuffer(winW, winH, sX, sY);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	rv = GFraMe_opengl_createSpriteTexture(texF, texW, texH);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	glClearColor(0, 0, 0, 0);
	
__ret:
	return rv;
}

void GFraMe_opengl_clear() {
	glDeleteTextures(1, &GFraMe_bbTexture);
	glDeleteBuffers(1, &GFraMe_bbVao);
	glDeleteFramebuffers(1, &GFraMe_bbFbo);
	glDeleteBuffers(1, &GFraMe_bbVbo);
	
	glDeleteProgram(GFraMe_ppProgram);
	glDeleteProgram(GFraMe_program);
	
	glDeleteTextures(1, &GFraMe_spriteTex);
	glDeleteBuffers(1, &GFraMe_spriteVao);
	glDeleteBuffers(1, &GFraMe_spriteVbo);
	glDeleteBuffers(1, &GFraMe_spriteIbo);
	
	if (GFraMe_opengl_ctx)
		SDL_GL_DeleteContext(GFraMe_opengl_ctx);
}

void GFraMe_opengl_setAtt() {
	//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void GFraMe_opengl_prepareRender() {
/**/
	glBindFramebuffer(GL_FRAMEBUFFER, GFraMe_bbFbo); 
/**/
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(GFraMe_program);
	
	glBindVertexArray(GFraMe_spriteVao);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GFraMe_spriteTex);
	//glBindSampler(GFraMe_sampler, GL_TEXTURE0);
/**/
}

void GFraMe_opengl_renderSprite(int x, int y, int d, int tx, int ty) {
/**/
	glUniform2f(GFraMe_translation, (float)x, (float)y);
	glUniform1f(GFraMe_dimension, (float)d);
	glUniform2f(GFraMe_texOffset, (float)tx, (float)ty);
	
#if defined(GFRAME_DEBUG)
	GFraMe_opengl_validateProgram(GFraMe_program);
#endif
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
/**/
}

void GFraMe_opengl_doRender() {
/**/
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// remove those for awesomeness!!
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(GFraMe_ppProgram);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GFraMe_bbTexture);
	//glBindSampler(GFraMe_ppSampler, GL_TEXTURE0+1);
	
	glBindVertexArray(GFraMe_bbVao);
	
#if defined(GFRAME_DEBUG)
	GFraMe_opengl_validateProgram(GFraMe_ppProgram);
#endif
	
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	
	glBindVertexArray(0);
/**/
	glUseProgram(0);
	SDL_GL_SwapWindow(GFraMe_screen_get_window());
}

static void GFraMe_opengl_createSpriteBuffers() {
	const float vbo_data[] = {-0.5f,-0.5f, -0.5f,0.5f, 0.5f,0.5f, 0.5f,-0.5f};
	//const float vbo_data[] = {-1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f, 1.0f,-1.0f};
	const GLshort ibo_data[] = {0,1,2, 2,3,0};
	
	// Create a vertex buffer object (i.e., geometry vertexes)
	glGenBuffers(1, &GFraMe_spriteVbo);
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_spriteVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Create a index buffer object (i.e., map for vertexes)
	glGenBuffers(1, &GFraMe_spriteIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GFraMe_spriteIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_data), ibo_data,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static GFraMe_ret GFraMe_opengl_createSpriteTexture(char *file, int width,
	int height) {
	GFraMe_ret rv;
	char *data = NULL;
	
	// Load a texture
	rv = GFraMe_assets_buffer_image(file, width, height, &data);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to read file",
		rv = GFraMe_ret_failed, __ret);
	
	// Load a texture
	glGenTextures(1, &GFraMe_spriteTex);
	glBindTexture(GL_TEXTURE_2D, GFraMe_spriteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	rv = GFraMe_ret_ok;
__ret:
	if (data)
		free(data);
	return rv;
}

static void GFraMe_opengl_createVertexArrayObject() {
	glGenVertexArrays(1, &GFraMe_spriteVao);
	
	glBindVertexArray(GFraMe_spriteVao);
	
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_spriteVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GFraMe_spriteIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
	glBindVertexArray(0);
}

static GFraMe_ret GFraMe_opengl_createBackbuffer(int w, int h, int sX, int sY) {

	GFraMe_ret rv;
	GLenum status;
	GLfloat fbo_vertices[] = {-1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f, 1.0f,-1.0f};
	
	glGenTextures(1, &GFraMe_bbTexture);
	
	GFraMe_opengl_resetScreen(0, 0, w, h, sX, sY);
	
	glGenFramebuffers(1, &GFraMe_bbFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, GFraMe_bbFbo);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		GFraMe_bbTexture, 0);
	
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		GFraMe_new_log("FUCK!\n");
		return 1;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glGenBuffers(1, &GFraMe_bbVbo);
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_bbVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenVertexArrays(1, &GFraMe_bbVao);
	glBindVertexArray(GFraMe_bbVao);
	
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_bbVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GFraMe_spriteIbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
	rv = GFraMe_ret_ok;
__ret:
	return rv;
}

static void GFraMe_opengl_resetScreen(float x, float y, float w, float h,
	float sX, float sY) {
/*
	// TODO set those?
	this->_defX = X;
	this->_defY = Y;
	this->_defWidth = Width;
	this->_defHeight = Height;
	this->_defScaleX = ScaleX;
	this->_defScaleY = ScaleY;
	
	this->x = X;
	this->y = Y;
	this->scrollX = 0;
	this->scrollY = 0;
	this->width = Width;
	this->height = Height;
	this->scaleX = ScaleX;
	this->scaleY = ScaleY;
*/
	
	glBindTexture(GL_TEXTURE_2D, GFraMe_bbTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ((int)w) / sX, ((int)h) / sY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GFraMe_worldMatrix[0] = 2.0f / (float)w;
	GFraMe_worldMatrix[5] = -2.0f / (float)h;
	
	glUseProgram(GFraMe_ppProgram);
	glUniform2f(GFraMe_ppWindowDimensions, (float)w, (float)h);
	
	glUseProgram(GFraMe_program);
	glUniformMatrix4fv(GFraMe_locToGL, 1, GL_FALSE, GFraMe_worldMatrix);
	glUseProgram(0);
}

static GFraMe_ret GFraMe_opengl_createShaders() {
	GFraMe_ret rv;
	char *shaders[2], *ppShaders[2];
	GLenum shaderTypes[2], ppShaderTypes[2];
	
	shaders[0] = (char *)GFraMe_vertexShader;
	shaders[1] = (char *)GFraMe_fragmentShader;
	shaderTypes[0] = GL_VERTEX_SHADER;
	shaderTypes[1] = GL_FRAGMENT_SHADER;
	
	ppShaders[0] = (char *)GFraMe_vertexPPShader;
	ppShaders[1] = (char *)GFraMe_fragmentPPShader;
	ppShaderTypes[0] = GL_VERTEX_SHADER;
	ppShaderTypes[1] = GL_FRAGMENT_SHADER;
	
	GFraMe_program = GFraMe_opengl_createProgram(shaderTypes, shaders, 2);
	GFraMe_assertRV(GFraMe_program, "PROGRAM ERR", rv = GFraMe_ret_failed,
		__ret);
	GFraMe_ppProgram = GFraMe_opengl_createProgram(ppShaderTypes, ppShaders, 2);
	GFraMe_assertRV(GFraMe_ppProgram, "PROGRAM ERR", rv = GFraMe_ret_failed,
		__ret);
	
	GFraMe_locToGL = glGetUniformLocation(GFraMe_program, "locToGL");
	GFraMe_texDimensions = glGetUniformLocation(GFraMe_program,"texDimensions");
	GFraMe_translation = glGetUniformLocation(GFraMe_program, "translation");
	GFraMe_dimension = glGetUniformLocation(GFraMe_program, "dimension");
	GFraMe_texOffset = glGetUniformLocation(GFraMe_program, "texOffset");
	GFraMe_sampler = glGetUniformLocation(GFraMe_program, "gSampler");
	
	GFraMe_ppSampler = glGetUniformLocation(GFraMe_ppProgram, "gPpSampler");
	GFraMe_ppWindowDimensions = glGetUniformLocation(GFraMe_ppProgram,
		"windowDimensions");
	GFraMe_ppTexDimensions = glGetUniformLocation(GFraMe_ppProgram,
		"ppTexDimensions");
	
	rv = GFraMe_ret_ok;
__ret:
	return rv;
}

static GLuint GFraMe_opengl_createProgram(GLenum shaderTypes[],
	char *shaders[], int num) {
	GLuint *shaderList = NULL;
	GLuint program;
	GLint status;
	int i;
	
	i = 0;
	while (i < num)
		GFraMe_assertRV(shaders[i++], "NULL shader", program = 0, __ret);
	
	shaderList = (GLuint *)malloc(sizeof(GLuint)*num);
	GFraMe_assertRV(shaderList, "MEM ERROR", program = 0, __ret);
	
	i = 0;
	while (i < num) {
		char *shader;
		GLenum type;
		
		shader = shaders[i];
		type = shaderTypes[i];
		
		shaderList[i] = GFraMe_opengl_compileShader(type, shader);
		GFraMe_assertRV(shaderList[i], "ERR", program = 0, __ret);
		i++;
	}
	
	program = glCreateProgram();
	i = 0;
	while (i < num)
		glAttachShader(program, shaderList[i++]);
	
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		GLchar *strInfoLog;
		
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		strInfoLog = (GLchar *)malloc(infoLogLength+1);
		GFraMe_assertRV(strInfoLog, "MEM ERR", program = 0, __ret);
		
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		GFraMe_new_log("Linker failure: %s\n", strInfoLog);
		
		free(strInfoLog);
	}
	
	i = 0;
	while (i < num) {
		GLuint shader = shaderList[i++];
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}
	
__ret:
	if (shaderList)
		free(shaderList);
	
	return program;
}

static GLuint GFraMe_opengl_compileShader(GLenum eShaderType,
	char *strShaderFile) {
	GLuint shader;
	GLint status;
	
	shader = glCreateShader(eShaderType);
	glShaderSource(shader, 1, (const GLchar**)&strShaderFile, NULL);
	
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		GLchar *strInfoLog;
		const char *strShaderType = NULL;
		
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		strInfoLog = (GLchar *)malloc(infoLogLength + 1);
		GFraMe_assertRV(strInfoLog, "SHADER ERR", shader = 0, __ret);
		
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		
		switch (eShaderType) {
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			//case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}
		
		GFraMe_new_log("Compile failure in %s shader:\n%s\n", strShaderType,
			strInfoLog);
		
		free(strInfoLog);
	}
__ret:
	return shader;
}


static GFraMe_ret GFraMe_opengl_loadFunctions() {
	GFraMe_ret rv;
	
	#define LOAD_PROC(type, func) \
		func = (type) SDL_GL_GetProcAddress( #func );
	
	LOAD_PROC(PFNGLUSEPROGRAMPROC, glUseProgram);
	LOAD_PROC(PFNGLUNIFORM2FPROC, glUniform2f);
	LOAD_PROC(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
	LOAD_PROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
	LOAD_PROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
	LOAD_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
#if defined(_WIN32) || defined(_WIN64)
	LOAD_PROC(PFNGLACTIVETEXTUREPROC, glActiveTexture);
#endif
	LOAD_PROC(PFNGLBINDSAMPLERPROC, glBindSampler);
	LOAD_PROC(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
	LOAD_PROC(PFNGLBINDBUFFERPROC, glBindBuffer);
	LOAD_PROC(PFNGLUNIFORM1IPROC, glUniform1i);
	LOAD_PROC(PFNGLUNIFORM1FPROC, glUniform1f);
	LOAD_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
	LOAD_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
	LOAD_PROC(PFNGLGENBUFFERSPROC, glGenBuffers);
	LOAD_PROC(PFNGLBUFFERDATAPROC, glBufferData);
	LOAD_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
	LOAD_PROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
	LOAD_PROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
	LOAD_PROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
	LOAD_PROC(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);
	LOAD_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
	LOAD_PROC(PFNGLCREATEPROGRAMPROC, glCreateProgram);
	LOAD_PROC(PFNGLATTACHSHADERPROC, glAttachShader);
	LOAD_PROC(PFNGLLINKPROGRAMPROC, glLinkProgram);
	LOAD_PROC(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
	LOAD_PROC(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
	LOAD_PROC(PFNGLDETACHSHADERPROC, glDetachShader);
	LOAD_PROC(PFNGLDELETESHADERPROC, glDeleteShader);
	LOAD_PROC(PFNGLCREATESHADERPROC, glCreateShader);
	LOAD_PROC(PFNGLSHADERSOURCEPROC, glShaderSource);
	LOAD_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);
	LOAD_PROC(PFNGLGETSHADERIVPROC, glGetShaderiv);
	LOAD_PROC(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
	LOAD_PROC(PFNGLVALIDATEPROGRAMPROC, glValidateProgram);
	
	rv = GFraMe_ret_ok;
//__ret:
	return rv;
/*
	shaders_supported = SDL_FALSE;
    if (SDL_GL_ExtensionSupported("GL_ARB_shader_objects") &&
        SDL_GL_ExtensionSupported("GL_ARB_shading_language_300") &&
        SDL_GL_ExtensionSupported("GL_ARB_vertex_shader") &&
        SDL_GL_ExtensionSupported("GL_ARB_fragment_shader")) {
		
        glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) SDL_GL_GetProcAddress("glAttachObjectARB");
        glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC) SDL_GL_GetProcAddress("glCompileShaderARB");
        glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC) SDL_GL_GetProcAddress("glCreateProgramObjectARB");
        glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC) SDL_GL_GetProcAddress("glCreateShaderObjectARB");
        glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC) SDL_GL_GetProcAddress("glDeleteObjectARB");
        glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) SDL_GL_GetProcAddress("glGetInfoLogARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) SDL_GL_GetProcAddress("glGetObjectParameterivARB");
        glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC) SDL_GL_GetProcAddress("glGetUniformLocationARB");
        glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) SDL_GL_GetProcAddress("glLinkProgramARB");
        glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) SDL_GL_GetProcAddress("glShaderSourceARB");
        glUniform1iARB = (PFNGLUNIFORM1IARBPROC) SDL_GL_GetProcAddress("glUniform1iARB");
        glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC) SDL_GL_GetProcAddress("glUseProgramObjectARB");
		
        if (glAttachObjectARB && glCompileShaderARB && glCreateProgramObjectARB
			&& glCreateShaderObjectARB && glDeleteObjectARB && glGetInfoLogARB
			&& glGetObjectParameterivARB && glGetUniformLocationARB
			&& glLinkProgramARB && glShaderSourceARB && glUniform1iARB
			&& glUseProgramObjectARB)
            shaders_supported = SDL_TRUE;
    }

    if (!shaders_supported) {
        return SDL_FALSE;
    }
*/
}

#if defined(GFRAME_DEBUG)
static void GFraMe_opengl_validateProgram(GLuint program) {
	GLint status;
	
	glValidateProgram(program);
	
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	
	if (status != GL_TRUE) {
		GLint infoLogLength;
		GLchar *strInfoLog;
		
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if (infoLogLength == 0) {
			GFraMe_new_log("Program error! (but no log)\n");
			return;
		}
		
		strInfoLog = (GLchar *)malloc(infoLogLength + 1);
		if (!strInfoLog) {
			GFraMe_new_log("PROGRAM ERROR!\n");
			return;
		}
		
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		
		GFraMe_new_log("Program error: %s\n", strInfoLog);
	}
}
#endif

#endif
