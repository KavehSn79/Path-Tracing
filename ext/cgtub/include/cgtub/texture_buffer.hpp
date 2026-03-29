#pragma once

#include <span>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace cgtub
{

class GLTextureBuffer
{
public:
    GLTextureBuffer();

    virtual ~GLTextureBuffer();

    // Delete move and copy constructors/operators
    GLTextureBuffer(GLTextureBuffer const&)              = delete;
    GLTextureBuffer(GLTextureBuffer&&)                   = delete;
    GLTextureBuffer&  operator=(GLTextureBuffer const&)  = delete;
    GLTextureBuffer& operator=(GLTextureBuffer&&)        = delete;

    void upload(std::span<glm::vec3 const> values, int width, int height);

    GLuint gl() const;

private:
    GLuint m_texture;
};

} // namespace cgtub