#pragma once

#include <span>

#include <glad/glad.h>

namespace cgtub
{

template<typename T>
class GLAttributeBuffer
{
public:
    GLAttributeBuffer();

    virtual ~GLAttributeBuffer();

    // Delete move and copy constructors/operators
    GLAttributeBuffer(GLAttributeBuffer const&)            = delete;
    GLAttributeBuffer(GLAttributeBuffer&&)                 = delete;
    GLAttributeBuffer& operator=(GLAttributeBuffer const&) = delete;
    GLAttributeBuffer& operator=(GLAttributeBuffer&&)    = delete;

    void upload(std::span<T const> values);

    size_t elements() const;

    GLuint gl() const;

private:
    GLuint m_buffer;
    size_t m_elements;
};

} // namespace cgtub