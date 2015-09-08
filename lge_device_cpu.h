#pragma once
#include "lge_system.h"
#include "scene.h"
#include "lge_device.h"
#include "lge_texcache.h"
#include <atomic>

class LGE_CPU_Device: public LGE_Device
{
public:
    LGE_CPU_Device();
    ~LGE_CPU_Device();
    bool SetSettings(LGE_DevSettings *pSettings);
    void SetRuntimeParams(LGE_RuntimeParams *pParams);
    void Render(unsigned int first_line, unsigned int count, float time);
    void WaitSync();
    void WaitRender();

    void InitGL();
    void RenderThread();

protected:
    LGE_DevSettings m_settings;
    LGE_RuntimeParams m_params;

    HANDLE *m_startEvents;
    HANDLE *m_doneEvents;
    HANDLE *m_pRenderThreads;
    struct ThreadParams
    {
        unsigned int m_firstLine;
        unsigned int m_count;
        float m_render_time;
    } *m_threadParams;

    glm::vec4 *m_screenBuf;

    std::atomic_uint m_threadIndex;
    unsigned int m_threads;
    float m_time;
    bool m_stop;
};

LGE_Device *LGE_CreateCPUDevice();
