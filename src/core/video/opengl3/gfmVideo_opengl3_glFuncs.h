/**
 * Export all pointers to OpenGL functions
 *
 * @file src/core/video/opengl3/gfmVideo_opengl3_glFuncs.h
 */
#ifndef __GFMVIDEO_GL3_GLFUNCS_H__
#define __GFMVIDEO_GL3_GLFUNCS_H__

#include <GFraMe/gfmError.h>

#include <SDL2/SDL_opengl.h>

/**
 * Load all required OpenGL functions
 */
extern gfmRV gfmVideo_GL3_glLoadFunctions();

extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
#if defined(_WIN32) || defined(_WIN64)
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

#endif /* __GFMVIDEO_GL3_GLFUNCS_H__ */

