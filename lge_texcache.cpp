#include "lge_texcache.h"

CTexturesCache::CTexturesCache()
{
    m_tex_ssbo_buf = 0;
    m_ssbo_buf_size = 0;
    m_commit_needed = false;
    m_textures.reserve(16);
    m_tex_handles.reserve(16);
}

CTexturesCache::~CTexturesCache()
{
    DeleteTexturesBlock(0, (unsigned int)m_textures.size());
}

unsigned int CTexturesCache::AllocTexture()
{
    GLuint texId = 0;
    glGenTextures(1, &texId);

    if (!m_free_gaps.empty())
    {
        std::map<unsigned int, bool>::iterator it = m_free_gaps.begin();
        m_textures[it->first] = texId;
        m_tex_handles[it->first] = 0;
        m_free_gaps.erase(it);
        return it->first;
    }
    if (m_textures.size() == m_textures.capacity())
    {
        m_textures.reserve(m_textures.capacity() * 2);
        m_tex_handles.reserve(m_textures.capacity() * 2);
    }
    m_textures.push_back(texId);
    m_tex_handles.push_back(0);
    m_commit_needed = true;
    return (unsigned int)(m_textures.size() - 1);
}

unsigned int CTexturesCache::AllocTexture(unsigned int width, unsigned int height, void *image)
{
    unsigned int tex = AllocTexture();
    glBindTexture(GL_TEXTURE_2D, m_textures[tex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    pglGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint64 texHandle = pglGetTextureHandleNV(m_textures[tex]);
    pglMakeTextureHandleResidentNV(texHandle);
    m_tex_handles[tex] = texHandle;
    return tex;
}

bool CTexturesCache::AllocTextureBlock(unsigned int count, unsigned int *texs)
{
    for (unsigned int i = 0; i < count; i++)
        texs[i] = AllocTexture();
    return true;
}

unsigned int CTexturesCache::LoadTexture(const char *filename)
{
    int tex = AllocTexture();
    return tex;
}

bool CTexturesCache::LoadToTexture(unsigned int tex, const char *filename)
{
    return true;
}

void CTexturesCache::DeleteTexture(unsigned int tex)
{
    glDeleteTextures(1, &m_textures[tex]);
    m_textures[tex] = 0;
    m_tex_handles[tex] = 0;
    m_free_gaps[tex] = true;
}

void CTexturesCache::DeleteTexturesBlock(unsigned int first, unsigned int count)
{
    for (unsigned int i = 0; i < count; i++)
        DeleteTexture(i);
}

void CTexturesCache::DeleteTexturesBuf(unsigned int *texs, unsigned int count)
{
    for (unsigned int i = 0; i < count; i++)
        DeleteTexture(texs[i]);
}

void CTexturesCache::CommitToGPU()
{
    if (!m_commit_needed || m_tex_handles.size() == 0)
        return;
    bool allocated = false;
    if (m_tex_handles.size() != m_ssbo_buf_size)
    {
        if (m_tex_ssbo_buf)
            pglDeleteBuffers(1, &m_tex_ssbo_buf);
        pglGenBuffers(1, &m_tex_ssbo_buf);
        allocated = true;
    }
    pglBindBuffer(GL_UNIFORM_BUFFER, m_tex_ssbo_buf);
    if (allocated)
        pglBufferData(GL_UNIFORM_BUFFER, m_tex_handles.size()*sizeof(GLuint64), &m_tex_handles[0], GL_DYNAMIC_DRAW);
    else
        pglBufferSubData(GL_UNIFORM_BUFFER, 0, m_tex_handles.size()*sizeof(GLuint64), &m_tex_handles[0]);
    pglBindBufferRange(GL_SHADER_STORAGE_BUFFER, TEX_SSBO_BINDING, m_tex_ssbo_buf, 0, m_tex_handles.size()*sizeof(GLuint64));
    pglBindBuffer(GL_UNIFORM_BUFFER, 0);
}
