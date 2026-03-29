#include "cgtub/attribute_buffer.hpp"

#include <typeinfo>

#include <glm/glm.hpp>

#include <cgtub/log.hpp>
#include <cgtub/gl_wrap.hpp>

namespace cgtub
{

template<typename T>
GLAttributeBuffer<T>::GLAttributeBuffer()
    : m_buffer(0u)
    , m_elements(0u)
{
    glGenBuffers(1, &m_buffer);
}

template<typename T>
GLAttributeBuffer<T>::~GLAttributeBuffer()
{
    glDeleteBuffers(1, &m_buffer);
}

template<typename T>
void GLAttributeBuffer<T>::upload(std::span<T const> values)
{
    if (values.empty())
    {
        log_message(LogLevel::Warn, "GLAttributeBuffer<%s>::upload: no values provided", typeid(T).name());
        return;
    }

    m_elements = values.size();

    // Query current size of the underlying GPU memory
    GLint64 byteSize{0u};
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &byteSize);

    // Resize the buffer if necessary
    // ATTENTION: might invalidate bindings? Test..
    if (byteSize < static_cast<GLint64>(values.size_bytes()))
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
        glBufferData(GL_ARRAY_BUFFER, values.size_bytes(), values.data(), GL_DYNAMIC_DRAW);
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, values.size_bytes(), values.data());
}

template<typename T>
size_t GLAttributeBuffer<T>::elements() const
{
    return m_elements;
}

template<typename T>
GLuint GLAttributeBuffer<T>::gl() const
{
    return m_buffer;
}

#define IMPLEMENT_ATTRIBUTE_BUFFER(type) \
    template class GLAttributeBuffer<type>;

IMPLEMENT_ATTRIBUTE_BUFFER(float)
IMPLEMENT_ATTRIBUTE_BUFFER(double)
IMPLEMENT_ATTRIBUTE_BUFFER(int)
IMPLEMENT_ATTRIBUTE_BUFFER(unsigned int)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::vec1)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dvec1)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::i32vec1)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::u32vec1)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::vec2)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dvec2)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::i32vec2)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::u32vec2)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::vec3)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dvec3)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::i32vec3)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::u32vec3)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::vec4)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dvec4)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::i32vec4)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::u32vec4)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::mat2)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::mat3)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::mat4)

IMPLEMENT_ATTRIBUTE_BUFFER(glm::dmat2)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dmat3)
IMPLEMENT_ATTRIBUTE_BUFFER(glm::dmat4)

} // namespace cgtub