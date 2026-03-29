#pragma once

#include <optional>
#include <string_view>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "cgtub/fwd.hpp"
#include "cgtub/primitives.hpp"

// Forward declarations
struct GLFWwindow;

namespace cgtub
{

void gl_check_error(const char* file, unsigned int line);

/**
 * \brief Create a window suitable for rendering.
 * 
 * \param[in]  width      Width of the window in pixels.       
 * \param[in]  height     Height of the window in pixels. 
 * \param[in]  title      Title of the window.
 * \param[out] window     The \c GLFWwindow object that represents the window. This is a function output.
 * \param[out] dispatcher \c EventDispatcher connected to the window that receives its events (resizing, user input, ...). This is a function output. 
 *  
 * \return True if the window was successfully created, false otherwise.
 */
bool init(unsigned int width, unsigned int height, std::string_view title, GLFWwindow** window, EventDispatcher** dispatcher);

// Release the window and its dispatcher (frees resources)
void uninit(GLFWwindow* window, EventDispatcher* dispatcher);

// Prepare a frame for rendering. Must be called at the start of the main loop.
void begin_frame(GLFWwindow* window);

// Finalize a frame. Must be called at the end of the main loop.
void end_frame(GLFWwindow* window);

void clear(GLFWwindow* window, float r, float g, float b, float a, std::optional<Rect> viewport = std::nullopt);

void set_viewport(GLFWwindow* window, std::optional<Rect> viewport = std::nullopt);

bool compile_shader(GLenum type, const char* source, GLuint* shader);

bool link_program(GLuint vshader, GLuint fshader, GLuint* program);

bool create_program(const char* vsource, const char* fsource, GLuint* program);

template<typename T>
void set_uniform(GLuint program, GLint location, T const& value);

template<typename T>
void bind_attribute(GLuint vao, GLint location, GLuint buffer);

void bind_texture(GLuint program, GLint location, GLuint texture);

template<typename T>
struct OpenGLType
{
};

template<>
struct OpenGLType<char>
{
    static constexpr int const value = GL_BYTE;
};

template<>
struct OpenGLType<unsigned char>
{
    static constexpr int const value = GL_BYTE;
};

template<>
struct OpenGLType<short>
{
    static constexpr int const value = GL_SHORT;
};

template<>
struct OpenGLType<unsigned short>
{
    static constexpr int const value = GL_UNSIGNED_SHORT;
};

template<>
struct OpenGLType<int>
{
    static constexpr int const value = GL_INT;
};

template<>
struct OpenGLType<unsigned int>
{
    static constexpr int const value = GL_UNSIGNED_INT;
};

template<>
struct OpenGLType<float>
{
    static constexpr int const value = GL_FLOAT;
};

template<>
struct OpenGLType<double>
{
    static constexpr int const value = GL_DOUBLE;
};

template<typename T>
struct OpenGLAttributeBinder
{
    static void bind_attribute(GLuint vao, GLint location, GLuint buffer)
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 1, OpenGLType<T>::value, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
};

template<glm::length_t L, typename T, enum glm::qualifier Q>
struct OpenGLAttributeBinder<glm::vec<L, T, Q>>
{
    static void bind_attribute(GLuint vao, GLint location, GLuint buffer)
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, L, OpenGLType<T>::value, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
};

template<glm::length_t C, glm::length_t R, typename T, enum glm::qualifier Q>
struct OpenGLAttributeBinder<glm::mat<C, R, T, Q>>
{
    static void bind_attribute(GLuint vao, GLint location, GLuint buffer)
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, C * R, OpenGLType<T>::value, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
};

template<typename T>
void bind_attribute(GLuint vao, GLint location, GLuint buffer)
{
    OpenGLAttributeBinder<T>::bind_attribute(vao, location, buffer);
}

} // namespace cgtub