
#if !defined(GFRAME_MOBILE)
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
#endif
