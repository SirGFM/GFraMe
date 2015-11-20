/**
 * OpenGL3 backend for the video functionalities
 * 
 * @file src/core/video/opengl3/gfmVideo_opengl3.h
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmUtils.h>

#include <GFraMe/core/gfmFile_bkend.h>

#include <GFraMe_int/core/gfmVideo_bkend.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>

#include <stdlib.h>
#include <string.h>

#include "gfmVideo_opengl3_glFuncs.h"

/** Define a texture array type */
gfmGenArr_define(gfmTexture);

struct stGFMTexture {
    /** The actual OpenGL texture */
    GLuint texture;
    /** Texture's width */
    int width;
    /** Texture's height */
    int height;
};

struct stGFMVideoGL3 {
/* ==== OPENGL FIELDS ======================================================= */
    SDL_GLContext *pGLCtx;
    GLfloat worldMatrix[16];
    GLint logSize;
    GLchar *pInfoLog;
/* ==== OPENGL SPRITE SHADER PROGRAM FIELDS ================================= */
    GLuint sprProgram;
    GLuint sprUnfTransformMatrix;
    GLuint sprUnfTexDimensions;
    GLuint sprUnfTexture;
    GLuint sprUnfInstanceData;
/* ==== OPENGL BACKBUFFER SHADER PROGRAM FIELDS ============================= */
    GLuint bbProgram;
    GLuint bbUnfTexture;
/* ==== OPENGL INSTANCED RENDERING FIELDS =================================== */
    /* Texture used to access the instance data on the shader */
    GLuint instanceTex;
    /* Buffer that store the instance data */
    GLuint instanceBuf;
    /* Number of batches rendered on this frame */
    int batchCount;
    /* Number of batches rendered on the last frame */
    int lastBatchCount;
    /* Number of sprites rendered on the last frame */
    int lastNumObjects;
    /* Number of sprites rendered on the current frame */
    int totalNumObjects;
    /* Number of sprites staged to render on this frame */
    int numObjects;
    /* Max number of sprites that can be rendered in a single batch */
    int maxObjects;
    /* Buffer, obtained from OpenGL, where each sprite date is written */
    GLint *pInstanceData;
/* ==== OPENGL DEFAULT MESH FIELDS ========================================== */
    GLuint meshVbo;
    GLuint meshIbo;
    GLuint meshVao;
/* ==== OPENGL BACKBUFFER FIELDS ============================================ */
    GLuint bbVbo;
    GLuint bbIbo;
    GLuint bbVao;
    GLuint bbTex;
    GLuint bbFbo;
/* ==== OPENGL RENDER FIELDS ================================================ */
    gfmTexture *pLastTexture;
/* ==== WINDOW FIELDS ======================================================= */
    /** Actual window (managed by SDL2) */
    SDL_Window *pSDLWindow;
    /** Device's width */
    int devWidth;
    /** Device's height */
    int devHeight;
    /** Window's width (useful only in windowed mode) */
    int wndWidth;
    /** Window's height (useful only in windowed mode) */
    int wndHeight;
    /** Current resolution (useful only in fullscreen) */
    int curResolution;
    /** Whether we are currently in full-screen mode */
    int isFullscreen;
    /** How many resolutions are supported by this device */
    int resCount;
/* ==== BACKBUFFER FIELDS =================================================== */
    /** Backbuffer's width */
    int bbufWidth;
    /** Backbuffer's height */
    int bbufHeight;
    /** Width of the actual rendered buffer */
    int scrWidth;
    /** Height of the actual rendered buffer */
    int scrHeight;
    /** Position of the rendered buffer on the window */
    int scrPosX;
    /** Position of the rendered buffer on the window */
    int scrPosY;
    /** Factor by which the (output) screen is bigger than the backbuffer */
    int scrZoom;
    /** Background red component */
    GLclampf bgRed;
    /** Background green component */
    GLclampf bgGreen;
    /** Background blue component */
    GLclampf bgBlue;
    /** Background alpha component */
    GLclampf bgAlpha;
/* ==== TEXTURE FIELDS ====================================================== */
    /** Every cached texture */
    gfmGenArr_var(gfmTexture, pTextures);
};
typedef struct stGFMVideoGL3 gfmVideoGL3;

/* Default shaders */
static char spriteVertexShader[] = 
#include "sprites_glsl.vs"
;

static char spriteFragmentShader[] =
#include "sprites_glsl.fs"
;

static char backbufferVertexShader[] = 
#include "bbuffer_glsl.vs"
;

static char backbufferFragmentShader[] = 
#include "bbuffer_glsl.fs"
;

/**
 * Set the background color
 * 
 * NOTE: This color is used only when cleaning the backbuffer. If the
 * backbuffer has to be letter-boxed into the window, it will cleaned with
 * black.
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_setBackgroundColor(gfmVideo *pVideo, int color) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Set the color */
    pCtx->bgAlpha = ((color >> 24) & 0xff) / 255.0f;
    pCtx->bgRed   = ((color >> 16) & 0xff) / 255.0f;
    pCtx->bgGreen = ((color >> 8) & 0xff) / 255.0f;
    pCtx->bgBlue  = (color & 0xff) / 255.0f;

    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Frees and cleans a previously allocated texture
 * 
 * @param  ppCtx The alocated texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static void gfmVideo_GL3_freeTexture(gfmTexture **ppCtx) {
    /* Check if the object was actually alloc'ed */
    if (ppCtx && *ppCtx) {
        /* Check if the texture was created and destroy it */
        if ((*ppCtx)->texture) {
            glDeleteTextures(1, &((*ppCtx)->texture));
            (*ppCtx)->texture = 0;
        }
        /* Free the memory */
        free(*ppCtx);

        *ppCtx = 0;
    }
}


/**
 * Initializes a new gfmVideo
 * 
 * @param  [out]ppCtx The alloc'ed gfmVideo context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED, ...
 */
static gfmRV gfmVideo_GL3_init(gfmVideo **ppCtx) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int didInit, irv;
    SDL_DisplayMode sdlMode;

    didInit = 0;
    pCtx = 0;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Alloc the video context */
    pCtx = (gfmVideoGL3*)malloc(sizeof(gfmVideoGL3));
    ASSERT(pCtx, GFMRV_ALLOC_FAILED);

    /* Clean the struct */
    memset(pCtx, 0x0, sizeof(gfmVideoGL3));

    /* Initialize the SDL2 video subsystem */
    irv = SDL_InitSubSystem(SDL_INIT_VIDEO);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Mark SDL2 as initialized, in case anything happens */
    didInit = 1;

    /* Get the device's default resolution */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, 0/*defResolution*/, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->devWidth = sdlMode.w;
    pCtx->devHeight = sdlMode.h;

    /* Retrieve the number of available resolutions */
    pCtx->resCount = SDL_GetNumDisplayModes(0);
    ASSERT(pCtx->resCount, GFMRV_INTERNAL_ERROR);

    /* Initialize the transformation matrix as intended: 
     * 1  0  0 -1
     * 0  1  0  1
     * 0  0  1  0
     * 0  0  0  1 */
    pCtx->worldMatrix[0] = 1;
    pCtx->worldMatrix[3] = -1;
    pCtx->worldMatrix[5] = 1;
    pCtx->worldMatrix[7] = 1;
    pCtx->worldMatrix[10] = 1;
    pCtx->worldMatrix[15] = 1;

    /* Set the return variables */
    *ppCtx = (gfmVideo*)pCtx;
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK) {
        /* On error, shutdown SDL and free the alloc'ed memory */
        if (didInit) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
        if (pCtx) {
            free(pCtx);
        }
    }

    return rv;
}

/**
 * Releases a previously alloc'ed/initialized gfmVideo
 * 
 * @param  [out]ppVideo The gfmVideo context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmVideo_GL3_free(gfmVideo **ppVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Sanitize arguments */
    ASSERT(ppVideo, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)*ppVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Clean all textures */
    gfmGenArr_clean(pCtx->pTextures, gfmVideo_GL3_freeTexture);

    if (pCtx->pInfoLog) {
        free(pCtx->pInfoLog);
    }

    /* Delete all OpenGL Objects */
    if (pCtx->meshVao) {
        glDeleteVertexArrays(1, &(pCtx->meshVao));
    }
    if (pCtx->meshIbo) {
        glDeleteBuffers(1, &(pCtx->meshIbo));
    }
    if (pCtx->meshVbo) {
        glDeleteBuffers(1, &(pCtx->meshVbo));
    }
    if (pCtx->bbFbo) {
        glDeleteFramebuffers(1, &(pCtx->bbFbo));
    }
    if (pCtx->bbTex) {
        glDeleteTextures(1, &(pCtx->bbTex));
    }
    if (pCtx->instanceTex) {
        glDeleteTextures(1, &(pCtx->instanceTex));
    }
    if (pCtx->bbVao) {
        glDeleteVertexArrays(1, &(pCtx->bbVao));
    }
    if (pCtx->bbIbo) {
        glDeleteBuffers(1, &(pCtx->bbIbo));
    }
    if (pCtx->bbVbo) {
        glDeleteBuffers(1, &(pCtx->bbVbo));
    }
    if (pCtx->instanceTex) {
        glDeleteTextures(1, &(pCtx->instanceTex));
        pCtx->instanceTex = 0;
    }
    if (pCtx->instanceBuf) {
        glDeleteBuffers(1, &(pCtx->instanceBuf));
        pCtx->instanceBuf = 0;
    }

    /* Delete the shader programs */
    if (pCtx->sprProgram) {
        glDeleteProgram(pCtx->sprProgram);
    }
    if (pCtx->bbProgram) {
        glDeleteProgram(pCtx->bbProgram);
    }

    /* Destroy the OpenGL context */
    if (pCtx->pGLCtx) {
        SDL_GL_DeleteContext(pCtx->pGLCtx);
    }

    /* Destroy the window */
    if (pCtx->pSDLWindow) {
        /* TODO revert screen to it's original mode? (is it required?) */
        SDL_DestroyWindow(pCtx->pSDLWindow);
    }

    /* Release the video context */
    free(pCtx);

    *ppVideo = 0;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Count how many resolution modes there are available when in fullscreen
 * 
 * @param  [out]pCount How many resolutions were found
 * @param  [ in]pVideo The video context (will store the resolutions list)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmVideo_GL3_countResolutions(int *pCount, gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the number of resolutions */
    *pCount = pCtx->resCount;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get one of the possibles window's resolution
 * 
 * If the resolutions hasn't been queried, this function will do so
 * 
 * @param  [out]pWidth   A possible window's width
 * @param  [out]pHeight  A possible window's height
 * @param  [out]pRefRate A possible window's refresh rate
 * @param  [ in]pVideo   The video context
 * @param  [ in]index    Resolution to be read (0 is the default resolution)
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                       GFMRV_INTERNAL_ERROR, GFMRV_INVALID_INDEX
 */
static gfmRV gfmVideo_GL3_getResolution(int *pWidth, int *pHeight,
        int *pRefRate, gfmVideo *pVideo, int index) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int irv;
    SDL_DisplayMode sdlMode;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pRefRate, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the resolution is valid */
    ASSERT(index < pCtx->resCount, GFMRV_INVALID_INDEX);

    /* Retrieve the dimensions for the current resolution mode */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, pCtx->curResolution, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Set the return */
    *pWidth = sdlMode.w;
    *pHeight = sdlMode.h;
    *pRefRate = sdlMode.refresh_rate;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Recalculate helper variables to render the backbuffer into a window
 *
 * @param  [ in]pCtx   The backbuffer context
 * @param  [ in]width  The window's width
 * @param  [ in]height The window's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
static gfmRV gfmVideoGL3_cacheDimensions(gfmVideoGL3 *pCtx, int width,
        int height) {
    gfmRV rv;
    int horRatio, verRatio;

    /* Check that the window's dimension is valid */
    ASSERT(width >= pCtx->bbufWidth, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    ASSERT(height >= pCtx->bbufHeight, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);

    /* Check each possible ratio */
    horRatio = (int)( (double)width / (double)pCtx->bbufWidth);
    verRatio = (int)( (double)height / (double)pCtx->bbufHeight);
    /* Get the smaller of the two */
    if (horRatio < verRatio)
        pCtx->scrZoom = horRatio;
    else
        pCtx->scrZoom = verRatio;
    ASSERT(pCtx->scrZoom > 0, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);

    /* Center the output */
    pCtx->scrPosX = (width - pCtx->bbufWidth * pCtx->scrZoom) / 2;
    pCtx->scrPosY = (height - pCtx->bbufHeight * pCtx->scrZoom) / 2;
    pCtx->scrWidth = pCtx->bbufWidth * pCtx->scrZoom;
    pCtx->scrHeight = pCtx->bbufHeight * pCtx->scrZoom;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Change the fullscreen resolution of the window
 * 
 * NOTE 1: The resolution is the index to one of the previously queried
 * resolutions
 * 
 * NOTE 2: This modification will only take effect when switching to
 * fullscreen mode
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]index  The resolution's index
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_INVALID_INDEX, GFMRV_WINDOW_NOT_INITIALIZED
 */
static gfmRV gfmVideo_GL3_setResolution(gfmVideo *pVideo, int index) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int irv;
    SDL_DisplayMode sdlMode;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the index is valid */
    ASSERT(index < pCtx->resCount, GFMRV_INVALID_INDEX);
    /* Check that the window was already initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Retrieve the desired mode */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, index, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Check if the backbuffer fit into this new window */
    ASSERT(sdlMode.w >= pCtx->bbufWidth, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    ASSERT(sdlMode.h >= pCtx->bbufHeight, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);

    /* Switch the fullscreen resolution */
    irv = SDL_SetWindowDisplayMode(pCtx->pSDLWindow, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    if (pCtx->isFullscreen) {
        /* Update helper variables */
        rv = gfmVideoGL3_cacheDimensions(pCtx, sdlMode.w, sdlMode.h);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Store the resolution */
    pCtx->curResolution = index;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Compile a GLSL shader
 * 
 * @param  [ in]pCtx       The video context
 * @param  [ in]shaderType The shader type
 * @param  [ in]pStr       The shader (must be NULL terminated)
 * @return                 The shader's identifier or 0, on error
 */
static GLuint gfmVideo_GL3_compileShader(gfmVideoGL3 *pCtx,  GLenum shaderType,
        char *pStr) {
    GLuint shader;
    GLint status;

    /* Create a new shader of the requested type */
    shader = glCreateShader(shaderType);
    /* Compile it from the giver string */
    glShaderSource(shader, 1, (const GLchar**)&pStr, NULL);
    glCompileShader(shader);
    /* Check if it compiled successfully */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        /* Retrieve the length of the error string */
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &(pCtx->logSize));
        /* Alloc enough bytes for the error string and retrieve it */
        pCtx->pInfoLog = (GLchar*)malloc(sizeof(GLchar) * pCtx->logSize);
        if (pCtx->pInfoLog) {
            glGetShaderInfoLog(shader, pCtx->logSize, &(pCtx->logSize),
                    pCtx->pInfoLog);
        }

        return 0;
    }

    return shader;
}

/**
 * Create a program from GLSL a vertex and a fragment shader
 * 
 * @param  [out]pProg    The compiled shader program
 * @param  [ in]pCtx     The video context
 * @param  [ in]pVShader The vertex shader
 * @param  [ in]pFShader The fragment shader
 * @return               GFMRV_OK, GFMRV_FRAGMENT_SHADER_ERROR,
 *                       GFMRV_VERTEX_SHADER_ERROR, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_glcreateProgram(GLuint *pProg, gfmVideoGL3 *pCtx,
        char *pVShader, char *pFShader) {
    gfmRV rv;
    GLuint vsi, fsi;
	GLuint program;
	GLint status;

    vsi = 0;
    fsi = 0;
    program = 0;
    *pProg = 0;

    /* Compile the vertex shader */
    vsi = gfmVideo_GL3_compileShader(pCtx, GL_VERTEX_SHADER, pVShader);
    ASSERT(vsi, GFMRV_VERTEX_SHADER_ERROR);

    /* Compile the fragment shader */
    fsi = gfmVideo_GL3_compileShader(pCtx, GL_FRAGMENT_SHADER, pFShader);
    ASSERT(fsi, GFMRV_VERTEX_SHADER_ERROR);

    /* Create the shader program */
    program = glCreateProgram();
    /* Attach both shaders to it and try to link it */
    glAttachShader(program, vsi);
    glAttachShader(program, fsi);
    glLinkProgram(program);
    /* Check that it linked successfully */
	glGetProgramiv(program, GL_LINK_STATUS, &status);
    ASSERT(status == GL_TRUE, GFMRV_INTERNAL_ERROR);

    /* Set the return variables */
    *pProg = program;
    rv = GFMRV_OK;
    /* Clear the program, so its no mistakenly destroyed */
    program = 0;
__ret:
    if (program) {
        glDetachShader(program, vsi);
        glDetachShader(program, fsi);
		glDeleteProgram(program);
    }
    if (*pProg) {
        glDetachShader(*pProg, vsi);
        glDetachShader(*pProg, fsi);
    }
    if (vsi) {
        glDeleteShader(vsi);
    }
    if (fsi) {
        glDeleteShader(fsi);
    }

    return rv;
}

/**
 * Load the game's default shaders
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_loadShaders(gfmVideoGL3 *pCtx) {
    gfmRV rv;

    /* TODO Load user-supplied shaders */

    /* Load the sprite program */
    rv = gfmVideo_GL3_glcreateProgram(&(pCtx->sprProgram), pCtx,
            spriteVertexShader, spriteFragmentShader);
    ASSERT(rv == GFMRV_OK, rv);
    /* Load the backbuffer program */
    rv = gfmVideo_GL3_glcreateProgram(&(pCtx->bbProgram), pCtx,
            backbufferVertexShader, backbufferFragmentShader);
    ASSERT(rv == GFMRV_OK, rv);

    /* Load the programs uniforms */
    pCtx->sprUnfTransformMatrix = glGetUniformLocation(pCtx->sprProgram,
            "locToGL");
    pCtx->sprUnfTexDimensions = glGetUniformLocation(pCtx->sprProgram,
            "texDimensions");
    pCtx->sprUnfTexture = glGetUniformLocation(pCtx->sprProgram, "gSampler");
    pCtx->sprUnfInstanceData = glGetUniformLocation(pCtx->sprProgram, "instanceData");
    pCtx->bbUnfTexture = glGetUniformLocation(pCtx->bbProgram, "gSampler");

    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK) {
        /* Delete any compile program, on error */
        if (pCtx->sprProgram) {
		    glDeleteProgram(pCtx->sprProgram);
        }
        if (pCtx->bbProgram) {
		    glDeleteProgram(pCtx->bbProgram);
        }
    }

    return rv;
}

/**
 * Create the OpenGL backbuffer
 * 
 * @param  [ in]pCtx   The video context
 * @param  [ in]width  The backbuffer's width
 * @param  [ in]height The backbuffer's height
 * @return             GFMRV_OK, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_createBackbuffer(gfmVideoGL3 *pCtx, int width,
        int height) {
    /* Backbuffer mesh */
    float bbVboData[] = {-1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f, 1.0f,-1.0f};
    /* Default mesh vertices (square centered on 0 of side 1) */
	float meshVboData[] = {-0.5f,-0.5f, -0.5f,0.5f, 0.5f,0.5f, 0.5f,-0.5f};
    /* Error value */
    gfmRV rv;
    /* Sequence used to render the mesh as two triangles */
    GLshort iboData[] = {0,1,2, 2,3,0};
    /* Used for GL error checking */
    GLenum status;
    /* Maximum number of texels in a buffer texture */
    GLint maxBufTexels;

    /* Create the in-video-memory (?) backbuffer mesh */
    glGenBuffers(1, &(pCtx->bbVbo));
    ASSERT(pCtx->bbVbo, GFMRV_INTERNAL_ERROR);
    /* Load the mesh data into the GPU (?) */
    glBindBuffer(GL_ARRAY_BUFFER, pCtx->bbVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bbVboData), bbVboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Create the index for the backbuffer mesh */
    glGenBuffers(1, &(pCtx->bbIbo));
    ASSERT(pCtx->bbIbo, GFMRV_INTERNAL_ERROR);
    /* Load the index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCtx->bbIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iboData), iboData,
            GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Create a vertex array that associates both mesh and indexes */
    glGenVertexArrays(1, &(pCtx->bbVao));
    ASSERT(pCtx->bbVao, GFMRV_INTERNAL_ERROR);
    /* Associate both objects with this vao */
    glBindVertexArray(pCtx->bbVao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, pCtx->bbVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCtx->bbIbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    /* Create the backbuffer actual texture */
    glGenTextures(1, &(pCtx->bbTex));
    ASSERT(pCtx->bbTex, GFMRV_INTERNAL_ERROR);
    glBindTexture(GL_TEXTURE_2D, pCtx->bbTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    /* Remove any ugly (i.e., non-nearest-neighbour) up/down-scale */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* Actually create the texture */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Create the framebuffer used to target the texture */
    glGenFramebuffers(1, &(pCtx->bbFbo));
    ASSERT(pCtx->bbFbo, GFMRV_INTERNAL_ERROR);
    /* Associate the framebuffer with the texture */
    glBindFramebuffer(GL_FRAMEBUFFER, pCtx->bbFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            pCtx->bbTex, 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ASSERT(status == GL_FRAMEBUFFER_COMPLETE, GFMRV_INTERNAL_ERROR);

    /* Create the default mesh */
    glGenBuffers(1, &(pCtx->meshVbo));
    ASSERT(pCtx->bbVbo, GFMRV_INTERNAL_ERROR);
    /* Load the mesh data into the GPU (?) */
    glBindBuffer(GL_ARRAY_BUFFER, pCtx->meshVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(meshVboData), meshVboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Create the index for the default mesh */
    glGenBuffers(1, &(pCtx->meshIbo));
    ASSERT(pCtx->meshIbo, GFMRV_INTERNAL_ERROR);
    /* Load the index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCtx->meshIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iboData), iboData,
            GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Create a vertex array that associates both mesh and indexes */
    glGenVertexArrays(1, &(pCtx->meshVao));
    ASSERT(pCtx->bbVao, GFMRV_INTERNAL_ERROR);
    /* Associate both objects with this vao */
    glBindVertexArray(pCtx->meshVao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, pCtx->meshVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCtx->meshIbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    /* TODO Make this number used defined */
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE , &maxBufTexels);
    if (maxBufTexels < 8192 * 2) {
        pCtx->maxObjects = maxBufTexels / 2;
    }
    else {
        pCtx->maxObjects = 8192; /* For 8192 objects, 192KB VRAM is needed */
    }
    /* Create the instance data buffer (used within the texture) */
    glGenBuffers(1, &(pCtx->instanceBuf));
    ASSERT(pCtx->instanceBuf, GFMRV_INTERNAL_ERROR);
    glBindBuffer(GL_TEXTURE_BUFFER, pCtx->instanceBuf);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(int) * pCtx->maxObjects * 2 * 3, 0,
            GL_STREAM_DRAW);

    /* Create a texture to pass data to the shader */
    glGenTextures(1, &(pCtx->instanceTex));
    ASSERT(pCtx->instanceTex, GFMRV_INTERNAL_ERROR);
    glBindTexture(GL_TEXTURE_BUFFER, pCtx->instanceTex);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* Bind the texture to the buffer */
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, pCtx->instanceBuf);

    /* Modify the transformation matrix */
	pCtx->worldMatrix[0] = 2.0f / (float)width;
	pCtx->worldMatrix[5] = -2.0f / (float)height;
    /* Store the backbbufer dimensions */
    pCtx->bbufWidth = width;
    pCtx->bbufHeight = height;

    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK) {
        /* Clean everything on error */
        if (pCtx->meshVao) {
            glDeleteVertexArrays(1, &(pCtx->meshVao));
            pCtx->meshVao = 0;
        }
        if (pCtx->meshIbo) {
            glDeleteBuffers(1, &(pCtx->meshIbo));
            pCtx->meshIbo = 0;
        }
        if (pCtx->meshVbo) {
            glDeleteBuffers(1, &(pCtx->meshVbo));
            pCtx->meshVbo = 0;
        }
        if (pCtx->bbFbo) {
            glDeleteFramebuffers(1, &(pCtx->bbFbo));
            pCtx->bbFbo = 0;
        }
        if (pCtx->bbTex) {
            glDeleteTextures(1, &(pCtx->bbTex));
            pCtx->bbTex = 0;
        }
        if (pCtx->bbVao) {
            glDeleteVertexArrays(1, &(pCtx->bbVao));
            pCtx->bbVao = 0;
        }
        if (pCtx->bbIbo) {
            glDeleteBuffers(1, &(pCtx->bbIbo));
            pCtx->bbIbo = 0;
        }
        if (pCtx->bbVbo) {
            glDeleteBuffers(1, &(pCtx->bbVbo));
            pCtx->bbVbo = 0;
        }
        if (pCtx->instanceTex) {
            glDeleteTextures(1, &(pCtx->instanceTex));
            pCtx->instanceTex = 0;
        }
        if (pCtx->instanceBuf) {
            glDeleteBuffers(1, &(pCtx->instanceBuf));
            pCtx->instanceBuf = 0;
        }
    }

    return rv;
}

/**
 * Create the only window for the game
 * 
 * NOTE 1: The window may switch to fullscreen mode later
 * 
 * NOTE 2: The window's dimensions shall be clamped to the device's ones.
 * The resolution (i.e., width X height X refresh rate) may only take effect
 * when in fullscreen mode, so, in order to set all that on init, use
 * instead gfmVideo_GL3_initFullscreen
 * 
 * NOTE 3: The argument 'isUserResizable' defines whether a user may
 * manually stretch/shrink, but doesn't control whether or not a window's
 * dimensions may be modified programatically
 * 
 * @param  [ in]pCtx       The video context
 * @param  [ in]width      The desired width
 * @param  [ in]height     The desired height
 * @param  [ in]bbufWidth  The backbuffer's width
 * @param  [ in]bbufHeight The backbuffer's height
 * @param  [ in]pName      The game's title
 * @param  [ in]flags      Whether the user can resize the window
 * @param  [ in]vsync      Whether vsync is enabled or not
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                         GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_createWindow(gfmVideoGL3 *pCtx, int width,
        int height, int bbufWidth, int bbufHeight, char *pName,
        SDL_WindowFlags flags, int vsync) {
    gfmRV rv;
    int irv;

    /* if pName is NULL, the window should have no title */
    if (!pName) {
        pName = "";
    }

    /* Clamp the dimensions to the device's */
    if (width > pCtx->devWidth) {
        width = pCtx->devWidth;
    }
    if (height > pCtx->devHeight) {
        height = pCtx->devHeight;
    }

    /* Check that the window is big enough to support the backbuffer */
    ASSERT(bbufWidth <= width, GFMRV_BACKBUFFER_WIDTH_INVALID);
    ASSERT(bbufHeight <= height, GFMRV_BACKBUFFER_HEIGHT_INVALID);

    /* Ready the OpenGL attributes */
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    /* SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1); */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);

    /* Create the window */
    pCtx->pSDLWindow = SDL_CreateWindow(pName, SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    ASSERT(pCtx->pSDLWindow, GFMRV_INTERNAL_ERROR);

    /* Retrieve the OpenGL context */
    pCtx->pGLCtx = SDL_GL_CreateContext(pCtx->pSDLWindow);
    ASSERT(pCtx->pSDLWindow, GFMRV_INTERNAL_ERROR);

    /* Enable VSync */
    if (vsync) {
        irv = SDL_GL_SetSwapInterval(1);
        ASSERT(irv >= 0, GFMRV_INTERNAL_ERROR);
    }

    /* Load all required OpenGL functions */
    rv = gfmVideo_GL3_glLoadFunctions();
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Enable alpha blending (?) */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_1D);

    /* Load shaders */
    rv = gfmVideo_GL3_loadShaders(pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /* Create the GL backbuffer */
    rv = gfmVideo_GL3_createBackbuffer(pCtx, bbufWidth, bbufHeight);
    ASSERT(rv == GFMRV_OK, rv);

    /* Store the window (in windowed mode) dimensions */
    pCtx->wndWidth = width;
    pCtx->wndHeight = height;
    /* Set it at the default resolution (since it's the default behaviour) */
    pCtx->curResolution = 0;

    /* Already send the transform matrix to its uniform */
    glUseProgram(pCtx->sprProgram);
    glUniformMatrix4fv(pCtx->sprUnfTransformMatrix, 1, GL_FALSE, pCtx->worldMatrix);
	glUseProgram(0);

    /* Update helper variables */
    rv = gfmVideoGL3_cacheDimensions(pCtx, width, height);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set the background color */
    rv = gfmVideo_GL3_setBackgroundColor((gfmVideo*)pCtx, 0xff000000);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK) {
        if (pCtx->pSDLWindow) {
            SDL_DestroyWindow(pCtx->pSDLWindow);
        }
    }

    return rv;
}

/**
 * Create the only window for the game
 * 
 * NOTE 1: The window may switch to fullscreen mode later
 * 
 * NOTE 2: The window's dimensions shall be clamped to the device's ones.
 * The resolution (i.e., width X height X refresh rate) may only take effect
 * when in fullscreen mode, so, in order to set all that on init, use
 * instead gfmVideo_GL3_initFullscreen
 * 
 * NOTE 3: The argument 'isUserResizable' defines whether a user may
 * manually stretch/shrink, but doesn't control whether or not a window's
 * dimensions may be modified programatically
 * 
 * @param  [ in]pVideo          The video context
 * @param  [ in]width           The desired width
 * @param  [ in]height          The desired height
 * @param  [ in]bbufWidth       The backbuffer's width
 * @param  [ in]bbufHeight      The backbuffer's height
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @param  [ in]vsync           Whether vsync is enabled or not
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_initWindow(gfmVideo *pVideo, int width, int height,
        int bbufWidth, int bbufHeight, char *pName, int isUserResizable,
        int vsync) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    SDL_WindowFlags flags;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(width <= 16384, GFMRV_ARGUMENTS_BAD);
    ASSERT(height <= 16384, GFMRV_ARGUMENTS_BAD);
    /* Check that it hasn't been initialized */
    ASSERT(!pCtx->pSDLWindow, GFMRV_WINDOW_ALREADY_INITIALIZED);

    /* Set the SDL flag */
    flags = SDL_WINDOW_OPENGL;
    if (isUserResizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    /* Actually create the window */
    rv = gfmVideo_GL3_createWindow(pCtx, width, height, bbufWidth, bbufHeight,
            pName, flags, vsync);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set it as in windowed mode */
    pCtx->isFullscreen = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create the only window for the game in fullscreen mode
 * 
 * NOTE 1: The resolution is the index to one of the previously queried
 * resolutions
 * 
 * NOTE 2: The window may switch to windowed mode later
 * 
 * @param  [ in]pVideo          The video context
 * @param  [ in]resolution      The desired resolution
 * @param  [ in]bbufWidth       The backbuffer's width
 * @param  [ in]bbufHeight      The backbuffer's height
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @param  [ in]vsync           Whether vsync is enabled or not
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR,
 *                              GFMRV_INVALID_INDEX
 */
static gfmRV gfmVideo_GL3_initWindowFullscreen(gfmVideo *pVideo,
        int resolution, int bbufWidth, int bbufHeight, char *pName,
        int isUserResizable, int vsync) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    SDL_WindowFlags flags;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(resolution >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the resolution is valid */
    ASSERT(resolution < pCtx->resCount, GFMRV_INVALID_INDEX);
    /* Check that it hasn't been initialized */
    ASSERT(!pCtx->pSDLWindow, GFMRV_WINDOW_ALREADY_INITIALIZED);

    /* Set the SDL flag */
    flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL;
    if (isUserResizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    /* Actually create the window */
    rv = gfmVideo_GL3_createWindow(pCtx, pCtx->devWidth, pCtx->devHeight,
            bbufWidth, bbufHeight, pName, flags, vsync);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set it as in fullscreen mode */
    pCtx->isFullscreen = 1;

    /* Set the current resolution */
    rv = gfmVideo_GL3_setResolution(pCtx, resolution);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the window's dimensions
 * 
 * This modification will only take effect when in windowed mode. If the
 * window is currently in fullscreen mode, the modification will be delayed
 * until the switch is made
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]width  The desired width
 * @param  [ in]height The desired height
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_NOT_INITIALIZED
 */
static gfmRV gfmVideo_GL3_setDimensions(gfmVideo *pVideo, int width,
        int height) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Clamp the dimensions to the device's */
    if (width > pCtx->devWidth) {
        width = pCtx->devWidth;
    }
    if (height > pCtx->devHeight) {
        height = pCtx->devHeight;
    }

    /* Check if the backbuffer fit into this new window */
    ASSERT(width >= pCtx->bbufWidth, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    ASSERT(height >= pCtx->bbufHeight, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);

    /* Try to change the dimensions */
    SDL_SetWindowSize(pCtx->pSDLWindow, width, height);

    if (!pCtx->isFullscreen) {
        /* Update helper variables */
        rv = gfmVideoGL3_cacheDimensions(pCtx, width, height);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Store the new  dimensions */
    pCtx->wndWidth = width;
    pCtx->wndHeight = height;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the window's dimensions
 * 
 * If the window is in fullscreen mode, retrieve the dimensions for the
 * current resolution
 * 
 * @param  [out]pWidth  The current width
 * @param  [out]pHeight The current height
 * @param  [ in]pVideo  The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                      GFMRV_WINDOW_NOT_INITIALIZED
 */
static gfmRV gfmVideo_GL3_getDimensions(int *pWidth, int *pHeight,
        gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int irv;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    if (!pCtx->isFullscreen) {
        /* Retrieve the window's dimensions */
        *pWidth = pCtx->wndWidth;
        *pHeight = pCtx->wndHeight;
    }
    else {
        SDL_DisplayMode sdlMode;

        /* Retrieve the dimensions for the current resolution mode */
        irv = SDL_GetDisplayMode(0 /*displayIndex*/, pCtx->curResolution,
                &sdlMode);
        ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

        *pWidth = sdlMode.w;
        *pHeight = sdlMode.h;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Swith the current window mode to fullscreen
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_MODE_UNCHANGED, GFMRV_WINDOW_NOT_INITIALIZED
 */
static gfmRV gfmVideo_GL3_setFullscreen(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int irv;
    SDL_DisplayMode sdlMode;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Check that the window isn't in fullscreen mode */
    ASSERT(!pCtx->isFullscreen, GFMRV_WINDOW_MODE_UNCHANGED);

    /* Retrieve the desired mode */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, pCtx->curResolution, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Try to make it fullscrren */
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullscreen = 1;

    /* Update helper variables */
    rv = gfmVideoGL3_cacheDimensions(pCtx, sdlMode.w, sdlMode.h);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Swith the current window mode to windowed
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_MODE_UNCHANGED, GFMRV_WINDOW_NOT_INITIALIZED
 */
static gfmRV gfmVideo_GL3_setWindowed(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int irv;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Check that the window isn't in windowed mode */
    ASSERT(pCtx->isFullscreen, GFMRV_WINDOW_MODE_UNCHANGED);

    /* Try to make it fullscrren */
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, 0);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullscreen = 0;

    /* Update helper variables */
    rv = gfmVideoGL3_cacheDimensions(pCtx, pCtx->wndWidth, pCtx->wndHeight);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the backbuffer's dimensions
 * 
 * @param  [out]pWidth  The width
 * @param  [out]pHeight The height
 * @param  [ in]pVideo  The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_getBackbufferDimensions(int *pWidth, int *pHeight,
        gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Retrieve the backbuffer's dimensions */
    *pWidth = pCtx->bbufWidth;
    *pHeight = pCtx->bbufHeight;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Convert a point in window-space to backbuffer-space
 * 
 * NOTE: Both pX and pY must be initialized with the window-space point
 * 
 * @param  [out]pX   The horizontal position, in backbuffer-space
 * @param  [out]pY   The vertical position, in backbuffer-space
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_windowToBackbuffer(int *pX, int *pY,
        gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Convert the space */
    *pX = (*pX - pCtx->scrPosX) / (float)pCtx->scrZoom;
    *pY = (*pY - pCtx->scrPosY) / (float)pCtx->scrZoom;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the rendering operation
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_drawBegin(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Set the clear color to black */
    glClearColor(pCtx->bgRed, pCtx->bgGreen, pCtx->bgBlue, pCtx->bgAlpha);
    /* Clear the backbuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, pCtx->bbFbo);
    glClear(GL_COLOR_BUFFER_BIT);
    /* Activate the sprite rendering shader */
	glUseProgram(pCtx->sprProgram);
    /* Set its dimensions to the backbuffer's */
	glViewport(0, 0, pCtx->bbufWidth, pCtx->bbufHeight);

    /* Bind the default mesh (since it won't change through all rendering) */
    glEnableVertexAttribArray(0);
    glBindVertexArray(pCtx->meshVao);

    /* Clear the last texture, so it's at least pushed once */
    pCtx->pLastTexture = 0;
    /* Clear the number of rendered objects */
    pCtx->numObjects = 0;
    pCtx->pInstanceData = 0;

    /* Update the number of rendered sprites */
    pCtx->lastNumObjects = pCtx->totalNumObjects;
    pCtx->totalNumObjects = 0;

    /* Update the number of batched draws */
    pCtx->lastBatchCount = pCtx->batchCount;
    pCtx->batchCount = 0;

    /* Bind the texture to the sampler */
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindBuffer(GL_TEXTURE_BUFFER, pCtx->instanceBuf);
    glBindTexture(GL_TEXTURE_BUFFER, pCtx->instanceTex);
    glUniform1i(pCtx->sprUnfInstanceData, 1);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Alloc more data to pass with the instaces data
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_getInstanceData(gfmVideoGL3 *pCtx) {
    GLbitfield flags;

    flags = 0;
    flags |= GL_MAP_WRITE_BIT;
    flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
    //flags |= GL_MAP_UNSYNCHRONIZED_BIT;

    /* Orphan the previous buffer data and retrieve a new one */
    glBindBuffer(GL_TEXTURE_BUFFER, pCtx->instanceBuf);
    pCtx->pInstanceData = (GLint*)glMapBufferRange(GL_TEXTURE_BUFFER, 0,
            pCtx->maxObjects * 2 * 3, flags);

    if (!pCtx->pInstanceData) {
        return GFMRV_INTERNAL_ERROR;
    }
    return GFMRV_OK;
}

/** 
 * Draw the current batch of sprites
 * 
 * @param  [ in]pCtx The video context
 */
static void gfmVideo_GL3_drawInstances(gfmVideoGL3 *pCtx) {
    /** Allow the instances data to be used by the shader */
    glBindBuffer(GL_TEXTURE_BUFFER, pCtx->instanceBuf);
    glUnmapBuffer(GL_TEXTURE_BUFFER);
    pCtx->pInstanceData = 0;

    /* Actually render it */
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, pCtx->numObjects);

    pCtx->numObjects = 0;
    pCtx->batchCount++;
}

/**
 * Draw a tile into the backbuffer
 * 
 * @param  [ in]pVideo    The video context
 * @param  [ in]pSset     Spriteset containing the tile
 * @param  [ in]x         Horizontal (top-left) position in screen-space
 * @param  [ in]y         Vertical (top-left) position in screen-space
 * @param  [ in]tile      Index of the tile
 * @param  [ in]isFlipped Whether the tile should be flipped
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_drawTile(gfmVideo *pVideo, gfmSpriteset *pSset,
        int x, int y, int tile, int isFlipped) {
    gfmTexture *pTex;
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    int width, height;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(tile >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Retrieve the spriteset's texture */
    rv = gfmSpriteset_getTexture(&pTex, pSset);
    ASSERT(rv == GFMRV_OK, rv);
    /* If the texture just changed, load it into the shader */
    if (pTex != pCtx->pLastTexture) {
        pCtx->pLastTexture = pTex;

        if (pCtx->numObjects > 0) {
            gfmVideo_GL3_drawInstances(pCtx);
        }

        /* Bind the texture dimensions */
        glUniform2f(pCtx->sprUnfTexDimensions, (float)pTex->width,
                (float)pTex->height);
        /* Bind the texture */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pTex->texture);
        glUniform1i(pCtx->sprUnfTexture, 0);
    }

    /* Get the tile's dimensions */
    rv = gfmSpriteset_getDimension(&width, &height, pSset);
    ASSERT_NR(rv == GFMRV_OK);

    if (!pCtx->pInstanceData) {
        rv = gfmVideo_GL3_getInstanceData(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
    }

    /* Set the sprite's parameters */
    pCtx->pInstanceData[pCtx->numObjects * 6] = x;
    pCtx->pInstanceData[pCtx->numObjects * 6 + 1] = y;
    pCtx->pInstanceData[pCtx->numObjects * 6 + 2] = isFlipped;
    pCtx->pInstanceData[pCtx->numObjects * 6 + 3] = width;
    pCtx->pInstanceData[pCtx->numObjects * 6 + 4] = height;
    pCtx->pInstanceData[pCtx->numObjects * 6 + 5] = tile;

    pCtx->numObjects++;
    if (pCtx->numObjects == pCtx->maxObjects) {
        gfmVideo_GL3_drawInstances(pCtx);
    }

    pCtx->totalNumObjects++;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw the borders of a rectangle into the backbuffer
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_drawRectangle(gfmVideo *pVideo, int x, int y,
        int width, int height, int color) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    unsigned char alpha, blue, green, red;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Check that the rectangle is inside the screen */
    if (x + width < 0) {
        return GFMRV_OK;
    }
    if (y + height < 0) {
        return GFMRV_OK;
    }
    if (x >= pCtx->bbufWidth) {
        return GFMRV_OK;
    }
    if (y >= pCtx->bbufHeight) {
        return GFMRV_OK;
    }

    /* Retrieve each color component */
    alpha = (color >> 24) & 0xff;
    red   = (color >> 16) & 0xff;
    green = (color >> 8) & 0xff;
    blue  = color & 0xff;

    /* TODO Implement this */
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw a solid rectangle into the backbuffer
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmVideo_GL3_drawFillRectangle(gfmVideo *pVideo, int x, int y,
        int width, int height, int color) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;
    unsigned char alpha, blue, green, red;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Check that the rectangle is inside the screen */
    if (x + width < 0) {
        return GFMRV_OK;
    }
    if (y + height < 0) {
        return GFMRV_OK;
    }
    if (x >= pCtx->bbufWidth) {
        return GFMRV_OK;
    }
    if (y >= pCtx->bbufHeight) {
        return GFMRV_OK;
    }

    /* Retrieve each color component */
    alpha = (color >> 24) & 0xff;
    red   = (color >> 16) & 0xff;
    green = (color >> 8) & 0xff;
    blue  = color & 0xff;

    /* TODO Implement this */
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the backbuffer's data (i.e., composite of everything rendered to it
 * since the last gfmBackbuffer_drawBegin)
 * 
 * NOTE 1: Data is returned as 24 bits colors, with 8 bits per color and
 *         RGB order
 * 
 * NOTE 2: This function must be called twice. If pData is NULL, pLen will
 *         return the necessary length for the buffer. If pData isn't NULL,
 *         then pLen must be the length of pData
 * 
 * @param  [out]pData  Buffer where the data should be retrieved (caller
 *                     allocated an freed)
 * @param  [out]pLen   Returns the buffer length, in bytes
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                     GFMRV_BUFFER_TOO_SMALL, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_getBackbufferData(unsigned char *pData, int *pLen,
        gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Finalize the rendering operation
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_GL3_drawEnd(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Check if there's anything else to render */
    if (pCtx->numObjects > 0) {
        gfmVideo_GL3_drawInstances(pCtx);
    }

    /* Switch to the default framebuffer */
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /* Set it's default color and clean it */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Activate the output program */
    glUseProgram(pCtx->bbProgram);
    glViewport(pCtx->scrPosX, pCtx->scrPosY, pCtx->scrWidth, pCtx->scrHeight);
    /* Set the backbuffer as the input texture */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pCtx->bbTex);
    glUniform1i(pCtx->bbUnfTexture, 0);
    glBindVertexArray(pCtx->bbVao);
    /* Draw it */
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    /* Finally, swap the buffers */
    glUseProgram(0);
    SDL_GL_SwapWindow(pCtx->pSDLWindow);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve information about the last frame
 * 
 * @param  [out]pBatched The number of batched draws
 * @param  [out]pNum     The number of sprites rendered
 * @param  [ in]pvideo   The video context
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_getDrawInfo(int *pBatched, int *pNum, gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNum, GFMRV_ARGUMENTS_BAD);
    ASSERT(pBatched, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->bbFbo, GFMRV_BACKBUFFER_NOT_INITIALIZED);

    /* Retrieve the info */
    *pBatched = pCtx->lastBatchCount;
    *pNum = pCtx->lastNumObjects;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Alloc a new texture
 * 
 * @param  [out]ppCtx The alocated texture
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmVideo_GL3_getNewTexture(gfmTexture **ppCtx) {
    gfmRV rv;

    /* Alloc the texture */
    *ppCtx = (gfmTexture*)malloc(sizeof(gfmTexture));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);

    /* Initialize the object */
    memset(*ppCtx, 0x0, sizeof(gfmTexture));

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a texture
 * 
 * @param  [ in]pCtx   The alocated texture
 * @param  [ in]pVideo The video context
 * @param  [ in]width  The texture's width
 * @param  [ in]height The texture's height
 * @param  [ in]pLog   The logger interface
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmVideoGL3_initTexture(gfmTexture *pCtx, gfmVideoGL3 *pVideo,
        int width, int height, gfmLog *pLog) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT_LOG(width > 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(height > 0, GFMRV_ARGUMENTS_BAD, pLog);
    /* Check the dimensions */
    ASSERT_LOG(gfmUtils_isPow2(width) == GFMRV_TRUE,
            GFMRV_TEXTURE_INVALID_WIDTH, pLog);
    ASSERT_LOG(gfmUtils_isPow2(height) == GFMRV_TRUE,
            GFMRV_TEXTURE_INVALID_HEIGHT, pLog);

    /* Create the texture */
    glGenTextures(1, &(pCtx->texture));
    ASSERT(pCtx->texture, GFMRV_INTERNAL_ERROR);
    /* Set the dimensions (so we can load its data, later) */
    pCtx->width = width;
    pCtx->height = height;

    rv = GFMRV_OK;
__ret:
    return rv;
}

#define BMP_OFFSET_POS 0x0a
#define BMP_WIDTH_POS  0x12
#define BMP_HEIGHT_POS 0x16
#define READ_UINT(buffer) \
        ((0xff & buffer[0]) | ((buffer[1]<<8) & 0xff00) | \
        ((buffer[2]<<16) & 0xff0000) | ((buffer[3]<<24) & 0xff000000))


/**
 * Loads a 24 bits bitmap file into a texture
 * 
 * NOTE: The image's dimensions must be power of two (e.g., 256x256)
 * 
 * @param  [out]pTex     Handle to the loaded texture
 * @param  [ in]pVideo   The video context
 * @param  [ in]pFile    The texture file
 * @param  [ in]colorKey 24 bits, RGB Color to be treated as transparent
 * @param  [ in]pLog     The logger interface
 */
static gfmRV gfmVideo_GL3_loadTextureBMP(int *pTex, gfmVideo *pVideo,
        gfmFile *pFile, int colorKey, gfmLog *pLog) {
    char *pData, pBuffer[4];
    gfmRV rv;
    gfmTexture *pTexture;
    gfmVideoGL3 *pCtx;
    /*int bytesInRow, height, i, irv, dataOffset, rowOffset, width; */
    int bytesInRow, i, irv, rowOffset;
    volatile int height, dataOffset, width;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Zero variable that must be cleaned on error */
    pData = 0;
    pTexture = 0;

    /* Sanitize arguments */
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pTex, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pFile, GFMRV_ARGUMENTS_BAD, pLog);

    /*  Check the file type */
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 2/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(pBuffer[0] == 'B', GFMRV_TEXTURE_NOT_BITMAP, pLog);
    ASSERT_LOG(pBuffer[1] == 'M', GFMRV_TEXTURE_NOT_BITMAP, pLog);

    /* Get the offset to the image's "data section" */
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_OFFSET_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    dataOffset = READ_UINT(pBuffer);

    /* Get the image's dimensions */
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_HEIGHT_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    height = READ_UINT(pBuffer);

    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_WIDTH_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);

    width = READ_UINT(pBuffer);

    rv = gfmLog_log(pLog, gfmLog_info, "Loading %ix%i image...", width, height);
    ASSERT(rv == GFMRV_OK, rv);

    /* Alloc the data array */
    pData = (char*)malloc(width*height*sizeof(char)*4);
    ASSERT_LOG(pData, GFMRV_ALLOC_FAILED, pLog);

    /* Calculate how many bytes there are in a row of pixels */
    bytesInRow = width * 3;
    rowOffset = bytesInRow % 4;
    bytesInRow += rowOffset;

    /* Buffer the data (in the desired format) */
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, dataOffset);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    /* Data is written starting by the last line */
    i = width * (height - 1);
    /* Start "alpha" with 0 (since the image is in RGB-24) */
    pBuffer[3] = 0;
    while (1) {
        /*int color, r, g, b, pos, n; */
        int r, g, b, pos, n;
        volatile int color;

        /* Read until the EOF */
        rv = gfmFile_readBytes(pBuffer, &n, pFile, 3/*numBytes*/);
        ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_FILE_EOF_REACHED, rv, pLog);
        if (rv == GFMRV_FILE_EOF_REACHED) {
            break;
        }

        /* Get the actual color */
        color = READ_UINT(pBuffer);
        /* Get each component */
        r = color & 0xff;
        g = (color >> 8) & 0xff;
        b = (color >> 16) & 0xff;

        /* Output the color */
        pos = i * 4;
        if (color == colorKey) {
            pData[pos + 0] = 0x00;
            pData[pos + 1] = 0x00;
            pData[pos + 2] = 0x00;
            pData[pos + 3] = 0x00;
        }
        else {
            pData[pos + 0] = (char)b & 0xff;
            pData[pos + 1] = (char)g & 0xff;
            pData[pos + 2] = (char)r & 0xff;
            pData[pos + 3] =   0xff;
        }

        /* Go to the next pixel in this row */
        i++;
        /* If a row was read, go back the current row and the "next" one */
        /* (actually the previous!) */
        if (i % width == 0) {
            i -= width * 2;
            /* Go to the next line on the file */
            if (rowOffset != 0) {
                rv = gfmFile_seek(pFile, rowOffset);
                ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            }
        }
    }

    /* Initialize the texture  */
    gfmGenArr_getNextRef(gfmTexture, pCtx->pTextures, 1/*incRate*/, pTexture,
            gfmVideo_GL3_getNewTexture);
    rv = gfmVideoGL3_initTexture(pTexture, pCtx, width, height, pLog);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);

    /* Load the data into texture */
    glBindTexture(GL_TEXTURE_2D, pTexture->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, pData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Get the texture's index */
    *pTex = gfmGenArr_getUsed(pCtx->pTextures);
    /* Push the texture into the array */
    gfmGenArr_push(pCtx->pTextures);

    rv = GFMRV_OK;
__ret:
    /* Release the buffer, as it was already loaded into the texture */
    if (pData)
        free(pData);

    if (rv != GFMRV_OK) {
        if (pTexture) {
            /* On error, clean the texture and remove it from the list */
            gfmVideo_GL3_freeTexture(&pTexture);
            gfmGenArr_pop(pCtx->pTextures);
        }
    }

    return rv;
}

/**
 * Retrieve a texture's pointer from its index
 * 
 * @param  [out]pTexture The texture
 * @param  [ in]pVideo   The video context
 * @param  [ in]handle   The texture's handle
 * @param  [ in]pLog     The logger interface
 * @return               GFMRV_OK, GFMRV_ARUMENTS_BAD, GFMRV_INVALID_INDEX
 */
static gfmRV gfmVideo_GL3_getTexture(gfmTexture **ppTexture, gfmVideo *pVideo,
        int handle, gfmLog *pLog) {
    gfmRV rv;
    gfmVideoGL3 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoGL3*)pVideo;

    /* Sanitize arguments */
    ASSERT(pLog, GFMRV_ARGUMENTS_BAD);
    ASSERT_LOG(pCtx, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(ppTexture, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(handle >= 0, GFMRV_ARGUMENTS_BAD, pLog);
    /* Check that the texture is valid */
    ASSERT_LOG(handle < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX,
            pLog);

    /* Return the texture */
    *ppTexture = gfmGenArr_getObject(pCtx->pTextures, handle);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieves a texture's dimensions
 * 
 * @param [out]pWidth  The texture's width
 * @param [out]pHeight The texture's height
 * @param [ in]pCtx    The texture
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmVideo_GL3_getTextureDimensions(int *pWidth, int *pHeight,
        gfmTexture *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    *pWidth = pCtx->width;
    *pHeight = pCtx->height;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Load all SDL2 video functions into the struct
 * 
 * @param  [ in]pCtx The video function context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVideo_GL3_loadFunctions(gfmVideoFuncs *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Simply copy all function pointer */
    pCtx->gfmVideo_init = gfmVideo_GL3_init;
    pCtx->gfmVideo_free = gfmVideo_GL3_free;
    pCtx->gfmVideo_countResolutions = gfmVideo_GL3_countResolutions;
    pCtx->gfmVideo_getResolution = gfmVideo_GL3_getResolution;
    pCtx->gfmVideo_initWindow = gfmVideo_GL3_initWindow;
    pCtx->gfmVideo_initWindowFullscreen = gfmVideo_GL3_initWindowFullscreen;
    pCtx->gfmVideo_setDimensions = gfmVideo_GL3_setDimensions;
    pCtx->gfmVideo_getDimensions = gfmVideo_GL3_getDimensions;
    pCtx->gfmVideo_setFullscreen = gfmVideo_GL3_setFullscreen;
    pCtx->gfmVideo_setWindowed = gfmVideo_GL3_setWindowed;
    pCtx->gfmVideo_setResolution = gfmVideo_GL3_setResolution;
    pCtx->gfmVideo_getBackbufferDimensions = gfmVideo_GL3_getBackbufferDimensions;
    pCtx->gfmVideo_windowToBackbuffer = gfmVideo_GL3_windowToBackbuffer;
    pCtx->gfmVideo_setBackgroundColor = gfmVideo_GL3_setBackgroundColor;
    pCtx->gfmVideo_loadTextureBMP = gfmVideo_GL3_loadTextureBMP;
    pCtx->gfmVideo_drawBegin = gfmVideo_GL3_drawBegin;
    pCtx->gfmVideo_drawTile = gfmVideo_GL3_drawTile;
    pCtx->gfmVideo_drawRectangle = gfmVideo_GL3_drawRectangle;
    pCtx->gfmVideo_drawFillRectangle = gfmVideo_GL3_drawFillRectangle;
    pCtx->gfmVideo_getBackbufferData = gfmVideo_GL3_getBackbufferData;
    pCtx->gfmVideo_drawEnd = gfmVideo_GL3_drawEnd;
    pCtx->gfmVideo_getTexture = gfmVideo_GL3_getTexture;
    pCtx->gfmVideo_getTextureDimensions = gfmVideo_GL3_getTextureDimensions;
    pCtx->gfmVideo_getDrawInfo = gfmVideo_GL3_getDrawInfo;

    rv = GFMRV_OK;
__ret:
    return rv;
}

