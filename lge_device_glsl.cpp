#include "lge_device_glsl.h"
#include "lge_shader.h"
#include "lge_shader_denoise.h"
#include "lge_convert_b2.h"

static THREAD_RET THRAPI render_thread(void *pDev)
{
    SetThreadName("LGE_GLSL_Render");
    ((LGE_GLSL_Device *)pDev)->RenderThread();
    return 0;
}

LGE_GLSL_Device::LGE_GLSL_Device()
{
    m_renderThread = 0;
    m_startEvent = CreateEvent(0, 0, 0, 0);
    m_syncEvent  = CreateEvent(0, 0, 0, 0);
    m_doneEvent  = CreateEvent(0, 0, 0, 0);
    memset(&m_settings, 0, sizeof(m_settings));
    memset(&m_params, 0, sizeof(m_params));

    m_stop = !LoadGLFuncs();
}

LGE_GLSL_Device::~LGE_GLSL_Device()
{
    if (m_renderThread)
    {
        m_stop = true;
        SetEvent(m_startEvent);
        WaitThread(m_renderThread);
    }
    CloseHandle(m_renderThread);
    CloseHandle(m_doneEvent);
    CloseHandle(m_syncEvent);
    CloseHandle(m_startEvent);
}

bool LGE_GLSL_Device::SetSettings(LGE_DevSettings *pSettings)
{
    if (m_stop)
        return false;
    m_settings = *pSettings;
    if (!m_settings.m_threadedGL)
        InitGL();
    else
        m_renderThread = CreateThread(0, 0, render_thread, this, 0, 0);
    return true;
}

#define WIDTH  640
#define HEIGHT 480

void LGE_GLSL_Device::SetRuntimeParams(LGE_RuntimeParams *pParams)
{
    if (!m_settings.m_threadedGL && (m_params.m_width != pParams->m_width || m_params.m_height != pParams->m_height))
    {
        glViewport(0, 0, pParams->m_width, pParams->m_height); GLCHK;
        /*m_shaderPrimary.set(); GLCHK;
        pglUniform3f(m_res_uniform, (GLfloat)pParams->m_width, (GLfloat)pParams->m_height, (GLfloat)pParams->m_width / pParams->m_height); GLCHK;
        pglUseProgram(0); GLCHK;*/
        //m_fbPrimary.resize(pParams->m_width, pParams->m_height);
    }
    m_params = *pParams;
}

void LGE_GLSL_Device::InitGL()
{
    const char *shader_hdr = "#version 430 compatibility\n"\
        "#extension GL_NV_bindless_texture : require\n"\
        "#extension GL_NV_gpu_shader5 : require\n"\
        "#define MAIN void main(void)\n"\
        "#define assert(n)\n"\
        "uniform vec3  iResolution;\n"\
        "uniform float iGlobalTime;\n"\
        /*"uniform float iChannelTime[4];\n"*/\
        /*"uniform vec4  iMouse;\n"*/\
        /*"uniform vec4  iDate;\n"*/\
        /*"uniform vec3  iChannelResolution[4];\n"*/\
        "layout(std430, binding = 2) buffer LGE_Samplers\n"\
        "{\n"\
        "    sampler2D lge_samplers[];\n"\
        "};\n"\
        "vec4 tex2d(unsigned int index, vec2 uv, float lod)\n"\
        "{\n"\
        "    if (index >= lge_samplers.length())\n"\
        "        return vec4(0.5, 0.5, 0.5, 1.0);\n"\
        "    return texture2D(lge_samplers[index], uv, lod);\n"\
        "}\n";

    std::string shader_code = std::string(shader_hdr) + std::string((const char *)g_lge_shader);

    m_shaderPrimary.init(shader_code.c_str());

    m_res_uniform  = m_shaderPrimary.uniform("iResolution");
    m_time_uniform = m_shaderPrimary.uniform("iGlobalTime");

    pglGenBuffers(1, &m_params_ubo); GLCHK;
    pglBindBuffer(GL_UNIFORM_BUFFER, m_params_ubo); GLCHK;
    pglBufferData(GL_UNIFORM_BUFFER, sizeof(m_params), &m_params, GL_DYNAMIC_DRAW); GLCHK;
    pglBindBufferRange(GL_UNIFORM_BUFFER, PARAMS_UBO_BINDING, m_params_ubo, 0, sizeof(m_params)); GLCHK;
    pglBindBuffer(GL_UNIFORM_BUFFER, 0); GLCHK;

    m_scene = convert_from_b2(m_settings.m_scene);

    pglGenBuffers(1, &m_bvh_ssbo); GLCHK;
    pglBindBuffer(GL_UNIFORM_BUFFER, m_bvh_ssbo); GLCHK;
    pglBufferData(GL_UNIFORM_BUFFER, m_scene->m_heapSize*sizeof(*m_scene->m_heap), m_scene->m_heap, GL_DYNAMIC_DRAW); GLCHK;
    pglBindBufferRange(GL_SHADER_STORAGE_BUFFER, BVH_SSBO_BINDING, m_bvh_ssbo, 0, m_scene->m_heapSize*sizeof(*m_scene->m_heap)); GLCHK;
    pglBindBuffer(GL_UNIFORM_BUFFER, 0); GLCHK;

    m_shaderDenoise.init("uniform vec3 iResolution;"
        "uniform sampler2D tex0;\n"
        "uniform sampler2D tex1;\n"
        "void main()\n"
        "{\n"
            "vec2 uv = gl_FragCoord.xy / iResolution.xy;\n"
            "gl_FragColor = mix(texture2D(tex0, uv), texture2D(tex1, uv), 0.5);\n"
        "}\n");

    m_fbPrimary.init(WIDTH, HEIGHT);
    m_fbSecondary.init(WIDTH/2, HEIGHT/2);

    for (size_t i = 0; i < m_scene->m_textures.size(); i++)
    {
        m_texCache.AllocTexture(m_scene->m_textures[i].m_width, m_scene->m_textures[i].m_height, m_scene->m_textures[i].m_pData);
    }

    if (!m_settings.m_enableVSync)
        pwglSwapIntervalEXT(0);
}

void LGE_GLSL_Device::RenderThread()
{
    wglMakeCurrent((HDC)m_settings.m_HDC, (HGLRC)m_settings.m_HGLRC);
    InitGL();
    m_shaderPrimary.set(); GLCHK;

    while (!m_stop)
    {
        ResetEvent(m_syncEvent);
        WaitForSingleObject(m_startEvent, INFINITE);
        m_texCache.CommitToGPU();
        SetEvent(m_syncEvent);
        pglUniform3f(m_res_uniform, (GLfloat)m_params.m_width, (GLfloat)m_params.m_height, (GLfloat)m_params.m_width / m_params.m_height);
        pglUniform1f(m_time_uniform, m_time);
        glViewport(0, 0, m_params.m_width, m_params.m_height);
        glRecti(1, 1, -1, -1);
        glFinish();
        SetEvent(m_doneEvent);
    }
}

void LGE_GLSL_Device::Render(unsigned int first_line, unsigned int count, float time)
{
    m_firstLine = first_line;
    m_count = count;
    m_time = time;

    if (m_settings.m_threadedGL)
    {
        SetEvent(m_startEvent);
        return;
    }

    m_shaderPrimary.set(); GLCHK;
    m_texCache.CommitToGPU();
    pglUniform1f(m_time_uniform, m_time);
    pglUniform3f(m_res_uniform, WIDTH, HEIGHT, (GLfloat)WIDTH / HEIGHT); GLCHK;
    //pglUniform3f(m_res_uniform, (GLfloat)m_params.m_width, (GLfloat)m_params.m_height, (GLfloat)m_params.m_width / m_params.m_height); GLCHK;
    pglBindBuffer(GL_UNIFORM_BUFFER, m_params_ubo);
    LGE_RuntimeParams params = m_params;
    params.m_spp = 1;
    pglBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(params), &params);
    m_fbPrimary.set();
    glRecti(1, 1, -1, -1);

    pglBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_params), &m_params);
    pglUniform3f(m_res_uniform, WIDTH/2, HEIGHT/2, (GLfloat)(WIDTH / 2) / (HEIGHT / 2)); GLCHK;
    m_fbSecondary.set();
    glRecti(1, 1, -1, -1);
    pglBindBuffer(GL_UNIFORM_BUFFER, 0);
    pglBindFramebuffer(GL_FRAMEBUFFER, 0); GLCHK;
    pglUseProgram(0); GLCHK;

    m_shaderDenoise.set(); GLCHK;
    pglUniform1i(m_shaderDenoise.uniform("tex0"), 0);
    pglUniform1i(m_shaderDenoise.uniform("tex1"), 1);
    pglUniform3f(m_shaderDenoise.uniform("iResolution"), m_params.m_width, m_params.m_height, 1); GLCHK;
    glViewport(0, 0, m_params.m_width, m_params.m_height); GLCHK;
    glBindTexture(GL_TEXTURE_2D, m_fbPrimary.m_framebufferTex); GLCHK;
    pglActiveTexture(GL_TEXTURE0); GLCHK;
    glBindTexture(GL_TEXTURE_2D, m_fbSecondary.m_framebufferTex); GLCHK;
    pglActiveTexture(GL_TEXTURE1); GLCHK;
    glRecti(1, 1, -1, -1);
    /*glEnable(GL_TEXTURE_2D); GLCHK;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(1, 0); glVertex2f( 1, -1);
    glTexCoord2f(1, 1); glVertex2f( 1,  1);
    glTexCoord2f(0, 1); glVertex2f(-1,  1);
    glEnd();*/
}

void LGE_GLSL_Device::WaitSync()
{
    if (!m_settings.m_threadedGL)
        return;
    WaitForSingleObject(m_syncEvent, INFINITE);
}

void LGE_GLSL_Device::WaitRender()
{
    if (!m_settings.m_threadedGL)
        return;
    WaitForSingleObject(m_doneEvent, INFINITE);
}

LGE_Device *LGE_CreateGLSLDevice()
{
    LGE_GLSL_Device *dev = new LGE_GLSL_Device();
    if (!dev)
        return nullptr;
    if (dev->m_stop)
    {
        delete dev;
        return nullptr;
    }
    return (LGE_Device *)dev;
}
