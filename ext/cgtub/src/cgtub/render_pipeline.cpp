#include "cgtub/render_pipeline.hpp"

#include <iostream>

#include <cgtub/log.hpp>

namespace cgtub
{

GLRenderPipeline::GLRenderPipeline(GLFWwindow* window)
    : m_window(window)
    , m_program(0u)
    , m_vao(0u)
{
    if (!window)
        log_message(LogLevel::Error, "GLRenderPipeline::GLRenderPipeline: parameter 'window' is null.");

    glGenVertexArrays(1, &m_vao);
}

GLRenderPipeline::~GLRenderPipeline()
{
    glDeleteVertexArrays(1, &m_vao);
    if (m_program > 0)
        glDeleteProgram(m_program);
}

bool GLRenderPipeline::set_shaders(std::string const& vertex_shader_code, std::string const& fragment_shader_code)
{
    GLuint program;
    bool   success = create_program(vertex_shader_code.c_str(), fragment_shader_code.c_str(), &program);

    if (!success)
    {
        log_message(LogLevel::Error, "GLRenderPipeline::set_shaders: shader compilation failed, keeping the current pipeline shaders.");
        return false;
    }

    // Delete the old program
    if (m_program != 0u)
    {
        log_message(LogLevel::Trace, "GLRenderPipeline::set_shaders: deleting current shader program '%d'.", m_program);
        glUseProgram(0);
        glDeleteProgram(m_program);
    }

    m_program = program;

    return true;
}

bool GLRenderPipeline::has_attribute(std::string const& name)
{
    RENDER_PIPELINE_CHECK_NO_PROGRAM("has_attribute");

    return glGetAttribLocation(m_program, name.c_str()) >= 0;
}

bool GLRenderPipeline::has_uniform(std::string const& name)
{
    RENDER_PIPELINE_CHECK_NO_PROGRAM("has_uniform");

    return glGetUniformLocation(m_program, name.c_str()) >= 0;
}

bool GLRenderPipeline::has_texture(std::string const& name)
{
    RENDER_PIPELINE_CHECK_NO_PROGRAM("has_texture");

    return glGetUniformLocation(m_program, name.c_str()) >= 0;
}

bool GLRenderPipeline::bind_texture(std::string const& name, GLTextureBuffer const& texture)
{
    RENDER_PIPELINE_CHECK_NO_PROGRAM("bind_texture")

    GLint location = glGetUniformLocation(m_program, name.c_str());

    if (location < 0)
    {
        log_message(LogLevel::Error, "GLRenderPipeline::bind_texture: Texture '%s' is not found in the shaders.", name.c_str());
        return false;
    }

    ::cgtub::bind_texture(m_program, location, texture.gl());

    return true;
}

void GLRenderPipeline::render_indexed(GLAttributeBuffer<glm::u32vec3> const& indices, std::optional<Rect> viewport)
{
    set_viewport(m_window, viewport);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(m_program);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.gl());

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(3 * indices.elements()), GL_UNSIGNED_INT, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

} // namespace cgtub