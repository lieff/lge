#pragma once
#include <GL/gl.h>
#include <GL/glext.h>
#ifdef _WIN32
#include <GL/wglext.h>
#endif

#ifndef GLFUNCS_EXT
#define GLFUNCS_EXT extern
#endif

#ifdef _DEBUG
void CheckGLErrors(const char *func = "", int line = 0);
#define GLCHK CheckGLErrors(__FUNCTION__, __LINE__);
#else
#define GLCHK
#endif

#define PARAMS_UBO_BINDING 1
#define TEX_SSBO_BINDING 2
#define BVH_SSBO_BINDING 3

GLFUNCS_EXT PFNGLCREATESHADERPROC pglCreateShader;
GLFUNCS_EXT PFNGLCREATEPROGRAMPROC pglCreateProgram;
GLFUNCS_EXT PFNGLSHADERSOURCEPROC pglShaderSource;
GLFUNCS_EXT PFNGLCOMPILESHADERPROC pglCompileShader;
GLFUNCS_EXT PFNGLGETSHADERIVPROC pglGetShaderiv;
GLFUNCS_EXT PFNGLGETPROGRAMIVPROC pglGetProgramiv;
GLFUNCS_EXT PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog;
GLFUNCS_EXT PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog;
GLFUNCS_EXT PFNGLATTACHSHADERPROC pglAttachShader;
GLFUNCS_EXT PFNGLLINKPROGRAMPROC pglLinkProgram;
GLFUNCS_EXT PFNGLUSEPROGRAMPROC pglUseProgram;

GLFUNCS_EXT PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation;
GLFUNCS_EXT PFNGLUNIFORM1IPROC pglUniform1i;
GLFUNCS_EXT PFNGLUNIFORM1FPROC pglUniform1f;
GLFUNCS_EXT PFNGLUNIFORM3FPROC pglUniform3f;
GLFUNCS_EXT PFNGLACTIVETEXTUREPROC pglActiveTexture;
GLFUNCS_EXT PFNGLGENERATEMIPMAPEXTPROC pglGenerateMipmap;
GLFUNCS_EXT PFNGLBINDBUFFERRANGEPROC pglBindBufferRange;
GLFUNCS_EXT PFNGLGENBUFFERSPROC pglGenBuffers;
GLFUNCS_EXT PFNGLDELETEBUFFERSPROC pglDeleteBuffers;
GLFUNCS_EXT PFNGLBINDBUFFERPROC pglBindBuffer;
GLFUNCS_EXT PFNGLBUFFERDATAPROC pglBufferData;
GLFUNCS_EXT PFNGLBUFFERSUBDATAPROC pglBufferSubData;

GLFUNCS_EXT PFNGLGENFRAMEBUFFERSPROC pglGenFramebuffers;
GLFUNCS_EXT PFNGLDELETEFRAMEBUFFERSPROC pglDeleteFramebuffers;
GLFUNCS_EXT PFNGLBINDFRAMEBUFFERPROC pglBindFramebuffer;
GLFUNCS_EXT PFNGLFRAMEBUFFERTEXTUREPROC pglFramebufferTexture;

GLFUNCS_EXT PFNGLGETTEXTUREHANDLENVPROC pglGetTextureHandleNV;
GLFUNCS_EXT PFNGLUNIFORMHANDLEUI64VNVPROC pglUniformHandleui64vNV;
GLFUNCS_EXT PFNGLMAKETEXTUREHANDLERESIDENTNVPROC pglMakeTextureHandleResidentNV;

GLFUNCS_EXT PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT;

bool LoadGLFuncs();

struct LGE_FrameBuffer
{
    LGE_FrameBuffer();
    ~LGE_FrameBuffer();
    void init(int width, int height);
    void resize(int width, int height);
    void set();

    GLuint m_framebuffer;
    GLuint m_framebufferTex;
};

struct LGE_Shader
{
    GLuint m_prog;
    GLuint m_shader;

    LGE_Shader();

    bool init(const char *pCode);
    void set();
    GLint uniform(const char *name);
};
