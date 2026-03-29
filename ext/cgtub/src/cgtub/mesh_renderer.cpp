#include "cgtub/mesh_renderer.hpp"

#include <cassert>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

#include "mesh_renderer_shaders.hpp"

namespace cgtub
{

MeshRenderer::MeshRenderer(GLFWwindow* window)
    : m_window(window)
{
    // Create two buffers (positions, normals, color, indices)
    GLuint buffers[4];
    glGenBuffers(static_cast<GLsizei>(std::size(buffers)), buffers);
    m_vbo = buffers[0];
    m_nbo = buffers[1];
    m_cbo = buffers[2];
    m_ibo = buffers[3];

    // Create the vertex array object
    glGenVertexArrays(1, &m_vao);

    // Create the program
    // TODO: error handling
    create_program(vertex_shader_source, fragment_shader_source, &m_programs.color);
    create_program(colorlit_vertex_shader_source, colorlit_fragment_shader_source, &m_programs.colorlit);
    create_program(vcolor_vertex_shader_source, vcolor_fragment_shader_source, &m_programs.vertexcolor);
    create_program(position_vertex_shader_source, vcolor_fragment_shader_source, &m_programs.position);
    create_program(vertex_shader_source, identifier_fragment_shader_source, &m_programs.identifier);
}

MeshRenderer::~MeshRenderer()
{
    // TODO: Remove resources
}

void MeshRenderer::update_buffers(MeshRenderData const& mesh)
{
    if (!mesh.normals.empty())
        assert(mesh.positions.size() == mesh.normals.size());
    if (!mesh.colors.empty())
        assert(mesh.positions.size() == mesh.colors.size());

    // Check if the capacity of the vertex buffers is sufficient and resize if necessary
    size_t  requiredByteSize = mesh.positions.size_bytes();
    GLint64 currentByteSize{0u};
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentByteSize);
    if (static_cast<GLint64>(requiredByteSize) > currentByteSize)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, requiredByteSize, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_nbo);
        glBufferData(GL_ARRAY_BUFFER, requiredByteSize, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, requiredByteSize, nullptr, GL_DYNAMIC_DRAW);
    }

    // Check if the capacity of the index buffer is sufficient and resize if necessary
    requiredByteSize = mesh.indices.size_bytes();
    currentByteSize  = 0u;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glGetBufferParameteri64v(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentByteSize);
    if (static_cast<GLint64>(requiredByteSize) > currentByteSize)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, requiredByteSize, nullptr, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.positions.size_bytes(), mesh.positions.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_nbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.normals.size_bytes(), mesh.normals.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.colors.size_bytes(), mesh.colors.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh.indices.size_bytes(), mesh.indices.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MeshRenderer::update_vertex_array_object(MeshRenderData const& mesh, GLuint program)
{
    glBindVertexArray(m_vao);

    GLint loc = glGetAttribLocation(program, "position_vs");
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    loc = glGetAttribLocation(program, "normal_vs");
    if (!mesh.normals.empty() && loc >= 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_nbo);
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    else if (loc >= 0)
    {
        glDisableVertexAttribArray(loc);
    }

    loc = glGetAttribLocation(program, "color_vs");
    if (!mesh.colors.empty() && loc >= 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    else if (loc >= 0)
    {
        glDisableVertexAttribArray(loc);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MeshRenderer::render(MeshRenderData const& mesh, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, MeshRenderParams const& params, std::optional<Rect> viewport)
{
    if (mesh.positions.empty())
    {
        log_message(LogLevel::Warn, "MeshRenderer::render(): Mesh '%s' does not have vertex positions.", std::string(mesh.name).c_str());
        return;
    }

    update_buffers(mesh);

    gl_check_error(__FILE__, __LINE__);

    // Activate a viewport for rendering
    set_viewport(m_window, viewport);

    glEnable(GL_DEPTH_TEST);

    //
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // Bind the correct program
    GLuint program{0u};
    if (params.mode == MeshRenderMode::Color)
    {
        program = m_programs.color;
    }
    else if (params.mode == MeshRenderMode::ColorLit)
    {
        program = m_programs.colorlit;
    }
    else if (params.mode == MeshRenderMode::VertexColor)
    {
        program = m_programs.vertexcolor;
    }
    else if (params.mode == MeshRenderMode::Position)
    {
        program = m_programs.position;
    }
    else if (params.mode == MeshRenderMode::Identifier)
    {
        program = m_programs.identifier;
    }

    update_vertex_array_object(mesh, program);

    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "model_view_matrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix * mesh.matrix));

    glUniformMatrix4fv(glGetUniformLocation(program, "projection_matrix"),
                       1, GL_FALSE, glm::value_ptr(projection_matrix));

    if (params.mode == MeshRenderMode::Position)
        glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"),
                           1, GL_FALSE, glm::value_ptr(mesh.matrix));

    if (params.mode == MeshRenderMode::Color)
        glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(params.color));

    if (params.mode == MeshRenderMode::ColorLit)
    {
        glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(params.color));
        glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_FALSE, glm::value_ptr(mesh.matrix));
    }

    if (params.mode == MeshRenderMode::Identifier)
        glUniform1i(glGetUniformLocation(program, "id"), mesh.id);

    glBindVertexArray(m_vao);

    if (mesh.indices.empty())
    {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.positions.size()));
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(3 * mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // Not yet implemented...
    }

    glBindVertexArray(0);

    gl_check_error(__FILE__, __LINE__);
}

void MeshRenderer::render(MeshRenderData const& mesh, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport)
{
    render(mesh, view_matrix, projection_matrix, MeshRenderParams{}, viewport);
}

} // namespace cgtub