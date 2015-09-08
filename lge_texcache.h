#pragma once
#include <map>
#include <vector>
#include "glfuncs.h"

class CTexturesCache
{
public:
    CTexturesCache();
    ~CTexturesCache();

    unsigned int AllocTexture();
    unsigned int AllocTexture(unsigned int width, unsigned int height, void *image);
    bool AllocTextureBlock(unsigned int count, unsigned int *texs);
    unsigned int LoadTexture(const char *filename);
    bool LoadToTexture(unsigned int tex, const char *filename);

    void DeleteTexture(unsigned int tex);
    void DeleteTexturesBlock(unsigned int first, unsigned int count);
    void DeleteTexturesBuf(unsigned int *texs, unsigned int count);

    void CommitToGPU();

    std::vector<GLuint> m_textures;
    std::vector<GLuint64> m_tex_handles;
    std::map<unsigned int, bool> m_free_gaps;

    GLuint m_tex_ssbo_buf;
    unsigned int m_ssbo_buf_size;
    bool m_commit_needed;
};
