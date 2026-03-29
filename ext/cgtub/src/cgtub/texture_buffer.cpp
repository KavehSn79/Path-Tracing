#include <cgtub/texture_buffer.hpp>

#include <cgtub/gl_wrap.hpp>
#include <cgtub/log.hpp>

namespace cgtub
{

GLTextureBuffer::GLTextureBuffer()
    : m_texture(0u)
{
    glGenTextures(1, &m_texture);
}

GLTextureBuffer::~GLTextureBuffer()
{
    glDeleteTextures(1, &m_texture);
}

void GLTextureBuffer::upload(std::span<glm::vec3 const> values, int width, int height)
{
    if (values.empty())
    {
        log_message(LogLevel::Warn, "GLTextureBuffer::upload: no color values provided");
        return;
    }

    if (width <= 0 || height <= 0)
    {
        log_message(LogLevel::Warn, "GLTextureBuffer::upload: texture width and height must be > 0 (got width=%d height=%d)", width, height);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, OpenGLType<glm::vec3::value_type>::value, values.data());
}

GLuint GLTextureBuffer::gl() const
{
    return m_texture;
}

} // namespace cgtub