#include "cgtub/ndc_renderer.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "cgtub/canvas.hpp"
#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"
#include "mesh_renderer_shaders.hpp"

namespace cgtub
{

constexpr const char* passthrough_vertex_shader_source = R"glsl(
    #version 330

    precision highp float;

    in vec4 position_vs;

    out vec3 position_view;

    void main()
    {
        gl_Position   = position_vs;

        position_view = position_vs.xyz / position_vs.w;
    }
)glsl";

NDCRenderer::NDCRenderer(Canvas& canvas)
    : m_canvas(canvas)
    , m_line_renderer(canvas.window())
{
    // Create two buffers (positions, indices)
    GLuint buffers[2];
    glGenBuffers(static_cast<GLsizei>(std::size(buffers)), buffers);
    m_vbo = buffers[0];
    m_ibo = buffers[1];

    // Create the vertex array object
    glGenVertexArrays(1, &m_vao);

    // Create the program
    // TODO: error handling
    create_program(passthrough_vertex_shader_source, colorlit_fragment_shader_source, &m_program);
}

NDCRenderer::~NDCRenderer()
{
    // TODO: Clean up
}

void NDCRenderer::update_buffers(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices)
{
    // Check if the capacity of the vertex buffers is sufficient and resize if necessary
    size_t  required_byte_size = positions.size_bytes();
    GLint64 current_byte_size{0u};
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &current_byte_size);
    if (static_cast<GLint64>(required_byte_size) > current_byte_size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, required_byte_size, nullptr, GL_DYNAMIC_DRAW);
    }

    // Check if the capacity of the index buffer is sufficient and resize if necessary
    required_byte_size = indices.size_bytes();
    current_byte_size  = 0u;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glGetBufferParameteri64v(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &current_byte_size);
    if (static_cast<GLint64>(required_byte_size) > current_byte_size)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, required_byte_size, nullptr, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size_bytes(), positions.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size_bytes(), indices.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void NDCRenderer::update_vertex_array_object(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices)
{
    glBindVertexArray(m_vao);

    GLint loc = glGetAttribLocation(m_program, "position_vs");
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void NDCRenderer::render_mesh(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices, glm::vec3 const& color)
{
    if (positions.empty())
    {
        log_message(LogLevel::Warn, "NDCRenderer::render(): Mesh does not have vertex positions.");
        return;
    }

    update_buffers(positions, indices);

    gl_check_error(__FILE__, __LINE__);

    // Activate a viewport for rendering
    set_viewport(m_canvas.window(), m_canvas.viewport());

    glEnable(GL_DEPTH_TEST);

    // Bind the correct program
    update_vertex_array_object(positions, indices);

    glUseProgram(m_program);

    glUniform3fv(glGetUniformLocation(m_program, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(m_vao);

    if (indices.empty())
    {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size()));
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(3 * indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(0);

    gl_check_error(__FILE__, __LINE__);
}

void NDCRenderer::render_lines(std::span<glm::vec4 const> points, std::span<glm::vec3 const> colors)
{
    m_line_buffer.resize(points.size());
    for (size_t i = 0; i < points.size(); i++)
    {
        m_line_buffer[i] = points[i] / points[i].w;
    }
    m_line_renderer.render(m_line_buffer, colors, glm::mat4(1), glm::mat4(1), m_canvas.viewport());
}

} // namespace cgtub