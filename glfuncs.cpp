#define GLFUNCS_EXT
#include "glfuncs.h"
#include <stdio.h>

#ifdef _DEBUG
void CheckGLErrors(const char *func, int line)
{
    int lasterror = glGetError();
    if (lasterror)
    {
        printf("OpenGL error in %s, %i: err=%i\n", func, line, lasterror); fflush(stdout);
    }
}
#endif

struct GLFuncEntry
{
    void **pfunc;
    const char *name;
};

const GLFuncEntry g_glfuncs[] =
{
    { (void**)&pglCreateShader, "glCreateShader" },
    { (void**)&pglCreateProgram, "glCreateProgram" },
    { (void**)&pglShaderSource, "glShaderSource" },
    { (void**)&pglCompileShader, "glCompileShader" },
    { (void**)&pglGetShaderiv, "glGetShaderiv" },
    { (void**)&pglGetProgramiv, "glGetProgramiv" },
    { (void**)&pglGetShaderInfoLog, "glGetShaderInfoLog" },
    { (void**)&pglGetProgramInfoLog, "glGetProgramInfoLog" },
    { (void**)&pglAttachShader, "glAttachShader" },
    { (void**)&pglLinkProgram, "glLinkProgram" },
    { (void**)&pglUseProgram, "glUseProgram" },

    { (void**)&pglGetUniformLocation, "glGetUniformLocation" },
    { (void**)&pglUniform1i, "glUniform1i" },
    { (void**)&pglUniform1f, "glUniform1f" },
    { (void**)&pglUniform3f, "glUniform3f" },
    { (void**)&pglActiveTexture, "glActiveTexture" },
    { (void**)&pglGenerateMipmap, "glGenerateMipmap" },
    { (void**)&pglBindBufferRange, "glBindBufferRange" },
    { (void**)&pglGenBuffers, "glGenBuffers" },
    { (void**)&pglDeleteBuffers, "glDeleteBuffers" },
    { (void**)&pglBindBuffer, "glBindBuffer" },
    { (void**)&pglBufferData, "glBufferData" },
    { (void**)&pglBufferSubData, "glBufferSubData" },
    { (void**)&pglGetTextureHandleNV, "glGetTextureHandleNV" },
    { (void**)&pglUniformHandleui64vNV, "glUniformHandleui64vNV" },
    { (void**)&pglMakeTextureHandleResidentNV, "glMakeTextureHandleResidentNV" },

    { (void**)&pglGenFramebuffers, "glGenFramebuffers" },
    { (void**)&pglDeleteFramebuffers, "glDeleteFramebuffers" },
    { (void**)&pglBindFramebuffer, "glBindFramebuffer" },
    { (void**)&pglFramebufferTexture, "glFramebufferTexture" },

    { (void**)&pwglSwapIntervalEXT, "wglSwapIntervalEXT" }
};

bool LoadGLFuncs()
{
    for (int i = 0; i < sizeof(g_glfuncs) / sizeof(g_glfuncs[0]); i++)
    {
        *g_glfuncs[i].pfunc = wglGetProcAddress(g_glfuncs[i].name);
        if (!*g_glfuncs[i].pfunc)
            return false;
    }
    return true;
}

LGE_FrameBuffer::LGE_FrameBuffer()
{
    m_framebuffer = 0;
    m_framebufferTex = 0;
}

LGE_FrameBuffer::~LGE_FrameBuffer()
{
    if (m_framebuffer)
        pglDeleteFramebuffers(1, &m_framebuffer); GLCHK;
    if (m_framebufferTex)
        glDeleteTextures(1, &m_framebufferTex); GLCHK;
}

void LGE_FrameBuffer::init(int width, int height)
{
    pglGenFramebuffers(1, &m_framebuffer); GLCHK;
    pglBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer); GLCHK;
    glGenTextures(1, &m_framebufferTex); GLCHK;
    glBindTexture(GL_TEXTURE_2D, m_framebufferTex); GLCHK;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, 0); GLCHK;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GLCHK;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GLCHK;
    pglFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_framebufferTex, 0); GLCHK;
    pglBindFramebuffer(GL_FRAMEBUFFER, 0); GLCHK;
    glBindTexture(GL_TEXTURE_2D, 0); GLCHK;
}

void LGE_FrameBuffer::resize(int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, m_framebufferTex); GLCHK;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); GLCHK;
    glBindTexture(GL_TEXTURE_2D, 0); GLCHK;
}

void LGE_FrameBuffer::set()
{
    pglBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer); GLCHK;
}

LGE_Shader::LGE_Shader()
{
    m_prog = 0;
    m_shader = 0;
}

bool LGE_Shader::init(const char *pCode)
{
    m_prog = pglCreateProgram(); GLCHK;
    m_shader = pglCreateShader(GL_FRAGMENT_SHADER); GLCHK;
    pglShaderSource(m_shader, 1, (const GLchar **)&pCode, 0); GLCHK;
    pglCompileShader(m_shader); GLCHK;
#ifdef _DEBUG
    GLint isCompiled = 0;
    pglGetShaderiv(m_shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        pglGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &maxLength);
        GLchar *errorLog = (GLchar *)alloca(maxLength);
        pglGetShaderInfoLog(m_shader, maxLength, &maxLength, &errorLog[0]);
        printf(errorLog);
        return false;
    }
#endif
    pglAttachShader(m_prog, m_shader); GLCHK;
    pglLinkProgram(m_prog); GLCHK;
#ifdef _DEBUG
    GLint isLinked = 0;
    pglGetProgramiv(m_prog, GL_LINK_STATUS, &isLinked); GLCHK;
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        pglGetProgramiv(m_prog, GL_INFO_LOG_LENGTH, &maxLength); GLCHK;
        GLchar *errorLog = (GLchar *)alloca(maxLength);
        pglGetProgramInfoLog(m_prog, maxLength, &maxLength, &errorLog[0]); GLCHK;
        printf(errorLog);
        return false;
    }
#endif
    return true;
}

void LGE_Shader::set()
{
    pglUseProgram(m_prog); GLCHK;
}

GLint LGE_Shader::uniform(const char *name)
{
    return pglGetUniformLocation(m_prog, name); GLCHK;
}
