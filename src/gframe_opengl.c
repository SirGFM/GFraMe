/**
 * @src/gframe_opengl.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_log.h>
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
static GLuint GFraMe_dimesion;
static GLuint GFraMe_texOffset;
static GLuint GFraMe_sampler;

static GLuint GFraMe_ppProgram;

static GLuint GFraMe_ppSampler;

static GLuint GFraMe_bbTexture;
static GLuint GFraMe_bbFbo;
static GLuint GFraMe_bbVbo;

static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLACTIVETEXTUREPROC glActiveTexture;
static PFNGLBINDSAMPLERPROC glBindSampler;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLBINDBUFFERPROC glBindBuffer;
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

/*
static PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
static PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
static PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
static PFNGLUNIFORM1IARBPROC glUniform1iARB;
static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
*/

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
  "uniform float dimesion;\n"           // sprite dimension (e.g., 16 -> 16x16)
  "uniform vec2 texOffset;\n"           // tile position on the texture
  
  "out vec2 texCoord0;\n"
  
  "void main() {\n"
  
  "  vec2 texCoord;\n"
  "  vec2 pos;\n"
  
  "  pos  = vtx;\n"                     // 
  "  pos *= dimesion;\n"                //
  "  pos += vec2(0.5f, 0.5f);\n"        // set the origin at the top left corner
  "  pos += translation;\n"             // 
  "  vec4 position = vec4(pos.x, pos.y,"// convert it to a vec4
  "                     -1.0f, 1.0f);\n"
  "  gl_Position = position*locToGL;\n" // output vertex to OpenGL
  //"  gl_position = vec4(vtx, -1.0f, 1.0f);\n"
  
  //"	 texCoord0 = vtx;\n"
  "  texCoord0 = vtx + vec2(0.5f,"      // now it's a square's vertex of side 1
                             "0.5f);\n" //and origin (0,0)
  "  texCoord0 *= texDimensions;\n"     // make it 1-pixel wide
  "  texCoord0 *= dimesion;\n"          // make it "1-tile" wide
  "  texCoord0 = texCoord+texOffset;\n" // select which tile to use
  "}\n";
/*
	"#version 330\n"
	"layout(location = 0) in vec2 _pos;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	
	"uniform mat4 locToGL;\n"
	"uniform vec2 scale;\n"
	"uniform vec2 translation;\n"
	"uniform vec2 rotation;\n"
	"uniform vec2 texoffset;\n"
	
	"out vec2 texCoord0;\n"
	
	"void main()\n"
	"{\n"
	"	vec2 pos = _pos;\n"
	"	mat2 rot;\n"
	"	rot[0].xy = vec2(rotation.x, -rotation.y);\n"
	"	rot[1].xy = rotation.yx;\n"
	"	pos *= scale;\n"
	"	pos *= rot;\n"
	"	pos += translation;\n"
	"	vec4 position = vec4(pos.x, pos.y, -1.0f, 1.0f);\n"
	"	gl_Position = position*locToGL;\n"
	"	texCoord0 = texCoord+texoffset;"
	"}\n";
*/

static const char GFraMe_fragmentShader[] = 
	"#version 330\n"
	"in vec2 texCoord0;\n"
	"out vec4 outputColor;\n"
	
	"uniform sampler2D gSampler;\n"
	
	"void main()\n"
	"{\n"
	"	outputColor = texture2D(gSampler, texCoord0.st);\n"
	//"	outputColor = vec4(1.0f,0.0f,0.0f,1.0f);\n"
	"}\n";

// Shaders for rendering the backbuffer to the screen (Post Process)
static const char GFraMe_vertexPPShader[] = 
	"#version 330\n"
	"layout(location = 0) in vec2 pos;\n"
	"out vec2 _texCoord;\n"
	
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(pos, -1.0f, 1.0f);\n"
	"	_texCoord = 0.5f * (pos + vec2(1.0f, 1.0f));\n"
	"	_texCoord.y *= -1.0f;\n"
	"}\n";

static const char GFraMe_fragmentPPShader[] = 
	"#version 330\n"
	"in vec2 _texCoord;\n"
	"out vec4 outputColor;\n"
	
	"uniform sampler2D gPpSampler;\n"
//	"uniform float offsetY;\n"
//	"uniform vec4 flashColor;\n"
//	"uniform lowp float flashT;\n"
//	"uniform lowp float fadeT;\n"
	
	"void main()\n"
	"{\n"
	"	vec2 texCoord = _texCoord;\n"
	
		// Shake effect
//	"	texCoord.t += offsetY;\n"
	"	lowp vec4 pixel = texture2D(gPpSampler, texCoord.st);\n"
	"	pixel.rb = pixel.br;\n"
		// Flash effect
//	"	pixel = pixel*(1-flashT) + flashT*flashColor;\n"
		// Fade effect
//	"	pixel = pixel*(1-fadeT) + fadeT*vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
	
	"	outputColor = pixel;\n"
	//"	outputColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);\n"
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

static void GFrame_opengl_validateProgram(GLuint program);

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
	
	GFraMe_opengl_createSpriteBuffers();
	
	rv = GFraMe_opengl_createSpriteTexture(texF, texW, texH);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	GFraMe_opengl_createVertexArrayObject();
	
	rv = GFraMe_opengl_createShaders();
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	glUseProgram(GFraMe_program);
	glUniform2f(GFraMe_texDimensions, 1.0f / texW, 1.0f / texH);
	glUseProgram(0);
	
	rv = GFraMe_opengl_createBackbuffer(winW, winH, sX, sY);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "OpenGL ctx ERR",
		rv = GFraMe_ret_failed, __ret);
	
	glClearColor(0, 0, 0, 0);
	
__ret:
	return rv;
}

void GFraMe_opengl_clear() {
	glDeleteTextures(1, &GFraMe_bbTexture);
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
	glBindFramebuffer(GL_FRAMEBUFFER, GFraMe_bbFbo); 
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(GFraMe_program);
	
	glBindVertexArray(GFraMe_spriteVao);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GFraMe_spriteTex);
	glBindSampler(GFraMe_sampler, GL_TEXTURE0);
}

void GFraMe_opengl_renderSprite(int x, int y, int d, int tx, int ty) {
	glUniform2f(GFraMe_translation, (float)x, (float)y);
	glUniform1f(GFraMe_dimesion, (float)d);
	glUniform2f(GFraMe_texOffset, (float)tx, (float)ty);
	
#if defined(GFRAME_DEBUG)
	GFrame_opengl_validateProgram(GFraMe_program);
#endif
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void GFraMe_opengl_doRender() {
	//glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// remove those for awesomeness!!
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(GFraMe_ppProgram);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GFraMe_bbTexture);
	glBindSampler(GFraMe_ppSampler, GL_TEXTURE1);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_bbVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
#if defined(GFRAME_DEBUG)
	GFrame_opengl_validateProgram(GFraMe_ppProgram);
#endif
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	//glBindTexture(GL_TEXTURE_2D, 0);
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
	glActiveTexture(GL_TEXTURE0);
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
	
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_spriteVbo);	
	
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GFraMe_spriteIbo);
	
	glBindVertexArray(0);
}

static GFraMe_ret GFraMe_opengl_createBackbuffer(int w, int h, int sX, int sY) {

	GFraMe_ret rv;
	//GLenum status;
	GLfloat fbo_vertices[] = {-1.0f,-1.0f, 1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f};
	
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &GFraMe_bbTexture);
	glBindTexture(GL_TEXTURE_2D, GFraMe_bbTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//glGenRenderbuffers(1, &(this->rbo_depth));
	GFraMe_opengl_resetScreen(0, 0, w, h, sX, sY);
	
	glGenFramebuffers(1, &GFraMe_bbFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, GFraMe_bbFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		GFraMe_bbTexture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo_depth);
	
	//status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//GFraMe_assertRV(status != GL_FRAMEBUFFER_COMPLETE, "BBUF ERR",
	//	rv = GFraMe_ret_failed, __ret);
	
	glBindFramebuffer(GL_FRAMEBUFFER, GFraMe_bbFbo);
	
	glGenBuffers(1, &GFraMe_bbVbo);
	glBindBuffer(GL_ARRAY_BUFFER, GFraMe_bbVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
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
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GFraMe_bbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)w, (int)h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GFraMe_worldMatrix[0] = 2.0f / (float)w;
	GFraMe_worldMatrix[5] = -2.0f / (float)h;
	
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
	GFraMe_dimesion = glGetUniformLocation(GFraMe_program, "dimesion");
	GFraMe_texOffset = glGetUniformLocation(GFraMe_program, "texOffset");
	GFraMe_sampler = glGetUniformLocation(GFraMe_program, "gSampler");
	
	GFraMe_ppSampler = glGetUniformLocation(GFraMe_ppProgram, "gPpSampler");
	
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
	LOAD_PROC(PFNGLACTIVETEXTUREPROC, glActiveTexture);
	LOAD_PROC(PFNGLBINDSAMPLERPROC, glBindSampler);
	LOAD_PROC(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
	LOAD_PROC(PFNGLBINDBUFFERPROC, glBindBuffer);
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
static void GFrame_opengl_validateProgram(GLuint program) {
	GLint status;
	
	glValidateProgram(program);
	
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	
	if (status != GL_TRUE) {
		GLint infoLogLength;
		GLchar *strInfoLog;
		
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		
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
