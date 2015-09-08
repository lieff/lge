#pragma once
#include "lge_system.h"
#include "lge_device.h"
#include "lge_texcache.h"

class LGE_GLSL_Device: public LGE_Device
{
    friend LGE_Device *LGE_CreateGLSLDevice();
public:
    LGE_GLSL_Device();
    ~LGE_GLSL_Device();
    bool SetSettings(LGE_DevSettings *pSettings);
    void SetRuntimeParams(LGE_RuntimeParams *pParams);
    void Render(unsigned int first_line, unsigned int count, float time);
    void WaitSync();
    void WaitRender();

    void InitGL();
    void RenderThread();

protected:
    CTexturesCache m_texCache;
    LGE_DevSettings m_settings;
    LGE_RuntimeParams m_params;

    HANDLE m_startEvent;
    HANDLE m_syncEvent;
    HANDLE m_doneEvent;
    HANDLE m_renderThread;

    LGE_FrameBuffer m_fbPrimary;
    LGE_FrameBuffer m_fbSecondary;
    LGE_Shader m_shaderPrimary;
    LGE_Shader m_shaderDenoise;

    GLint  m_res_uniform;
    GLint  m_time_uniform;
    GLuint m_params_ubo;
    GLuint m_bvh_ssbo;

    unsigned int m_firstLine;
    unsigned int m_count;
    float m_time;
    bool m_stop;
};

LGE_Device *LGE_CreateGLSLDevice();
