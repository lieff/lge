#include "lge_device_cpu.h"
#include "lge_shader.h"
#include "lge_convert_b2.h"

static THREAD_RET THRAPI render_thread(void *pDev)
{
    SetThreadName("LGE_CPU_Render");
    ((LGE_CPU_Device *)pDev)->RenderThread();
    return 0;
}

LGE_CPU_Device::LGE_CPU_Device()
{
    m_stop = false;
#ifdef _DEBUG
    m_threads = 1;
#else
    m_threads = 8;
#endif
    m_threadIndex = 0;
    m_pRenderThreads = (HANDLE *)malloc(m_threads*sizeof(HANDLE));
    m_startEvents    = (HANDLE *)malloc(m_threads*sizeof(HANDLE));
    m_doneEvents     = (HANDLE *)malloc(m_threads*sizeof(HANDLE));
    m_threadParams   = (ThreadParams *)malloc(m_threads*sizeof(ThreadParams));
    memset(m_threadParams, 0, m_threads*sizeof(ThreadParams));
    m_screenBuf = nullptr;
    for (unsigned int i = 0; i < m_threads; i++)
    {
        m_startEvents[i] = CreateEvent(0, 0, 0, 0);
        m_doneEvents[i]  = CreateEvent(0, 0, 0, 0);
        m_pRenderThreads[i] = CreateThread(0, 0, render_thread, this, 0, 0);
        m_threadParams[i].m_render_time = 1.0f;
        m_threadParams[i].m_count = 1;
    }
    memset(&m_settings, 0, sizeof(m_settings));
    memset(&m_params, 0, sizeof(m_params));
}

LGE_CPU_Device::~LGE_CPU_Device()
{
    if (m_pRenderThreads)
    {
        m_stop = true;
        for (unsigned int i = 0; i < m_threads; i++)
        {
            SetEvent(m_startEvents[i]);
            WaitThread(m_pRenderThreads[i]);
            CloseHandle(m_pRenderThreads[i]);
            CloseHandle(m_doneEvents[i]);
            CloseHandle(m_startEvents[i]);
        }
        free(m_pRenderThreads);
        free(m_doneEvents);
        free(m_startEvents);
        free(m_threadParams);
    }
}

bool LGE_CPU_Device::SetSettings(LGE_DevSettings *pSettings)
{
    m_settings = *pSettings;

    m_scene = convert_from_b2(m_settings.m_scene);

    return true;
}

void LGE_CPU_Device::SetRuntimeParams(LGE_RuntimeParams *pParams)
{
    unsigned int height = (pParams->m_height < m_threads) ? m_threads : pParams->m_height;
    if (m_params.m_width != pParams->m_width || m_params.m_height != height)
    {
        if (m_screenBuf)
            free(m_screenBuf);
        m_screenBuf = (glm::vec4 *)malloc(pParams->m_width*height*sizeof(glm::vec4));
    }
    m_params = *pParams;
    m_params.m_height = height;
}

void mainImage(glm::vec4 &gl_FragCoord, glm::vec4 &gl_FragColor);

glm::vec3 iResolution;
float iGlobalTime;

namespace LGE_BVH
{
    extern glm::vec4 *nodes;
}

namespace LGE_Params
{
    extern glm::vec4 m_cameraRight;
    extern glm::vec4 m_cameraUp;
    extern glm::vec4 m_cameraForward;
    extern glm::vec4 m_cameraPosition;
    extern glm::vec4 m_sunDirection;
    extern glm::vec4 m_sunColor;
    extern unsigned int m_width;
    extern unsigned int m_height;
    extern unsigned int m_spp;
    extern unsigned int m_skyTexture;
    extern float m_secondaryRaysMipmap;
    extern float m_FOV;
};

LGE_Scene *g_scene;

struct SSBO
{
    size_t SSBO::length();
};
size_t SSBO::length()
{
    return g_scene->m_textures.size();
}

glm::vec4 tex2d(unsigned int index, glm::vec2 uv, float lod)
{
    if (index >= g_scene->m_textures.size())
        return glm::vec4(0.0, 0.0, 0.0, 0.0);
    LGE_Texture &tex = g_scene->m_textures[index];
    unsigned int *buf = (unsigned int *)tex.m_pData;
    unsigned int rgb = buf[(((unsigned int)(uv.x*tex.m_width)) % tex.m_width) + (((unsigned int)(uv.y*tex.m_height)) % tex.m_height)*tex.m_width];
    glm::vec4 res = glm::vec4((rgb & 255)*(1.0/255.0), ((rgb >> 8) & 255)*(1.0 / 255.0), ((rgb >> 16) & 255)*(1.0 / 255.0), (rgb >> 24)*(1.0 / 255.0));
    return res;
}

void LGE_CPU_Device::RenderThread()
{
    unsigned int thr_idx = atomic_fetch_add(&m_threadIndex, 1);
    while (!m_stop)
    {
        WaitForSingleObject(m_startEvents[thr_idx], INFINITE);
        uint64_t startTime = GetTime();
        unsigned int firstLine = m_threadParams[thr_idx].m_firstLine;
        unsigned int count     = m_threadParams[thr_idx].m_count;
        unsigned int width     = m_params.m_width;

        glm::vec4 *pbuf = m_screenBuf + firstLine*width;
        for (unsigned int y = firstLine; y < (firstLine + count); y++)
            for (unsigned int x = 0; x < width; x++)
            {
                glm::vec4 gl_FragCoord = glm::vec4((float)x, (float)y, 0, 0);
                mainImage(gl_FragCoord, pbuf[0]);
                pbuf++;
            }
        m_threadParams[thr_idx].m_render_time = (float)(GetTime() - startTime);
        SetEvent(m_doneEvents[thr_idx]);
    }
}

void LGE_CPU_Device::Render(unsigned int first_line, unsigned int count, float time)
{
    m_time = time;

    iGlobalTime = m_time;
    iResolution = glm::vec3(m_params.m_width, (float)m_params.m_height, (float)m_params.m_width / m_params.m_height);

    LGE_Params::m_cameraRight    = m_params.m_cameraRight;
    LGE_Params::m_cameraUp       = m_params.m_cameraUp;
    LGE_Params::m_cameraForward  = m_params.m_cameraForward;
    LGE_Params::m_cameraPosition = m_params.m_cameraPosition;
    LGE_Params::m_sunDirection   = m_params.m_sunDirection;
    LGE_Params::m_sunColor       = m_params.m_sunColor;
    LGE_Params::m_width          = m_params.m_width;
    LGE_Params::m_height         = m_params.m_height;
    LGE_Params::m_spp            = m_params.m_spp;
    LGE_Params::m_skyTexture     = m_params.m_skyTexture;
    LGE_Params::m_secondaryRaysMipmap = m_params.m_secondaryRaysMipmap;
    LGE_Params::m_FOV            = m_params.m_FOV;

    iResolution = glm::vec3(m_params.m_width, m_params.m_height, (float)m_params.m_width / m_params.m_height);
    g_scene = m_scene;
    LGE_BVH::nodes = (glm::vec4*)m_scene->m_heap;

    float totalTime = 0.0f;
    unsigned int totalLines = 0;
    for (unsigned int i = 0; i < m_threads; i++)
    {
        totalTime += m_threadParams[i].m_render_time;
        totalLines += m_threadParams[i].m_count;
    }
    float predicted_time = totalTime / totalLines * count;

    for (unsigned int i = 0; i < m_threads; i++)
    {
        unsigned int predict_lines;
        m_threadParams[i].m_firstLine = first_line;
        if (i == (m_threads - 1))
            predict_lines = count;
        else if (totalTime < 0.0001f)
            predict_lines = count / (m_threads - i);
        else
            predict_lines = (unsigned int)((predicted_time / m_threads) * (m_threadParams[i].m_count / m_threadParams[i].m_render_time) + 0.5f);
        if (predict_lines == 0)
            predict_lines = 1;
        if (predict_lines > (m_params.m_height - first_line - (m_threads - i - 1)))
            predict_lines = m_params.m_height - first_line - (m_threads - i - 1);
        m_threadParams[i].m_count = predict_lines;
        SetEvent(m_startEvents[i]);
        first_line += predict_lines;
        if (count > predict_lines)
            count -= predict_lines;
        else
            count = 1;
    }
}

void LGE_CPU_Device::WaitSync()
{
}

#include "lge_shader_denoise.cpp"

void LGE_CPU_Device::WaitRender()
{
    for (unsigned int i = 0; i < m_threads; i++)
    {
        WaitForSingleObject(m_doneEvents[i], INFINITE);
    }
    glRasterPos2i(-1, -1);
    Denoise(m_screenBuf, m_params.m_width, m_params.m_height);
    glDrawPixels(m_params.m_width, m_params.m_height, GL_RGBA, GL_FLOAT, m_screenBuf);
/*#define DIV 1000000.0
    printf("Load: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", m_threadParams[0].m_render_time / DIV, m_threadParams[1].m_render_time / DIV, m_threadParams[2].m_render_time / DIV, m_threadParams[3].m_render_time / DIV,
        m_threadParams[4].m_render_time / DIV, m_threadParams[5].m_render_time / DIV, m_threadParams[6].m_render_time / DIV, m_threadParams[7].m_render_time / DIV);
    printf("Load: %d %d %d %d %d %d %d %d\n", m_threadParams[0].m_count, m_threadParams[1].m_count, m_threadParams[2].m_count, m_threadParams[3].m_count,
        m_threadParams[4].m_count, m_threadParams[5].m_count, m_threadParams[6].m_count, m_threadParams[7].m_count);*/
}

LGE_Device *LGE_CreateCPUDevice()
{
    return new LGE_CPU_Device();
}
