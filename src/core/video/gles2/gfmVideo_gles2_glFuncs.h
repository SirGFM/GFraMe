/**
 * Export all pointers to OpenGL functions
 *
 * @file src/core/video/gles2/gfmVideo_gles2_glFuncs.h
 */
#ifndef __GFMVIDEO_GLES2_GLFUNCS_H__
#define __GFMVIDEO_GLES2_GLFUNCS_H__

#include <GFraMe/gfmError.h>

#include <SDL2/SDL_opengl.h>

/**
 * Load all required OpenGL functions
 */
extern gfmRV gfmVideo_GLES2_glLoadFunctions();

extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3IPROC glUniform3i;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
#if defined(_WIN32) || defined(_WIN64)
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLUNIFORM1IPROC glUniform1i;
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
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

#endif /* __GFMVIDEO_GLES2_GLFUNCS_H__ */

