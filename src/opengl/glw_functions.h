#include "glw_funcs_impl.h"

static void glw_loadFunctions() {

#if !defined(GFRAME_MOBILE)
	#define LOAD_PROC(type, func) \
		func = (type) SDL_GL_GetProcAddress( #func );
#else
	#define LOAD_PROC(foo, bar)
#endif
	
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
}

static GLuint compileShader(GLenum eShaderType,
	char *strShaderFile) {
	GLuint shader;
	GLint status;
	
	shader = glCreateShader(eShaderType);
	glShaderSource(shader, 1, (const GLchar**)&strShaderFile, NULL);
	
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		return 0;
	}
	
	return shader;
}


static GLuint createProgram(GLenum shaderTypes[],
	char *shaders[], int num) {
	GLuint *shaderList = NULL;
	GLuint program = 0;
	GLint status;
	int i;
	
	i = 0;
	while (i < num) {
		if (!shaders[i++])
			goto __ret;
	}
	
	shaderList = (GLuint *)malloc(sizeof(GLuint)*num);
	if (!shaderList)
		goto __ret;
	
	i = 0;
	while (i < num) {
		char *shader;
		GLenum type;
		
		shader = shaders[i];
		type = shaderTypes[i];
		
		shaderList[i] = compileShader(type, shader);
		if (!shaderList[i])
			goto __ret;
		i++;
	}
	
	program = glCreateProgram();
	i = 0;
	while (i < num)
		glAttachShader(program, shaderList[i++]);
	
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
		goto __ret;
	
	i = 0;
	while (i < num) {
		GLuint shader = shaderList[i++];
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}
	
__ret:
	if (shaderList)
		free(shaderList);
	if (status == GL_FALSE)
		return 0;
	
	return program;
}


