#pragma once
#include "lge_scene.h"

struct LGE_DevSettings
{
    LGE_Scene *m_scene;
    void *m_HDC;
    void *m_HGLRC;
    bool m_threadedGL;
    bool m_enableVSync;
};

struct LGE_RuntimeParams
{
    glm::vec4 m_cameraRight;
    glm::vec4 m_cameraUp;
    glm::vec4 m_cameraForward;
    glm::vec4 m_cameraPosition;
    glm::vec4 m_sunDirection;
    glm::vec4 m_sunColor;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_spp;
    unsigned int m_skyTexture;
    float m_secondaryRaysMipmap;
    float m_FOV;
};

class LGE_Device
{
public:
    LGE_Device() { m_scene = 0; };
    virtual ~LGE_Device() {};
    virtual bool SetSettings(LGE_DevSettings *pSettings) = 0;
    virtual void SetRuntimeParams(LGE_RuntimeParams *pParams) = 0;
    virtual void Render(unsigned int first_line, unsigned int count, float time)=0;
    virtual void WaitSync()=0;
    virtual void WaitRender()=0;

protected:
    LGE_Scene *m_scene;
};

LGE_Device *LGE_CreateDevice();
