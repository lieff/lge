#include "lge_device_cpu.h"
#include "lge_device_glsl.h"

LGE_Device *LGE_CreateDevice()
{
    LGE_Device *dev = LGE_CreateGLSLDevice();
    if (!dev)
        dev = LGE_CreateCPUDevice();
    return dev;
}
