#include "cgtub/point_renderer.hpp"

#include <utility>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

constexpr const char* vertex_shader_source = R"glsl(
    #version 330

    uniform mat4 view_matrix;
    uniform mat4 projection_matrix;

    uniform float two_over_width;
    uniform float two_over_height;
    uniform float point_size;

    layout (location = 0) in vec3 point;
    layout (location = 1) in vec3 color;

    out VertexData
    {
        vec3 color;
    }
    vs_out;

    void main()
    {
        // Generate a quad from vertex ids (avoids binding vertex data)
        // The mapping from vertex id to quad position is
        // 2 ___ 3     (0,1) ___ (1,1) 
        //  |   |           |   |
        //  |___|           |___|
        // 0     1     (0,0)     (1,0)
        vec2 uv = vec2(gl_VertexID & 1, (gl_VertexID & 2) >> 1);
        uv.x -= 0.5f;
        uv.y -= 0.5f;

        // Transform the point to ndc
        vec4 point_ndc = projection_matrix * view_matrix * vec4(point, 1);
        
        uv.x *= two_over_width  * point_size;
        uv.y *= two_over_height * point_size;

        // Shift the quad to the line origin (start point)
        vec2 origin = point_ndc.xy / point_ndc.w;
        uv += origin;

        gl_Position = vec4(uv.x * point_ndc.w, uv.y * point_ndc.w, point_ndc.z, point_ndc.w);
        vs_out.color = color;
    }
)glsl";

constexpr const char* fragment_shader_source = R"glsl(
    #version 330

    in VertexData
    {
        vec3 color;
    }
    fs_in;

    out vec4 color;

    void main()
    {
        color = vec4(fs_in.color, 1.0);
    }
)glsl";


PointRenderer::PointRenderer(GLFWwindow* window)
    : m_window(window)
{
    // Create two buffers (positions and color)
    GLuint buffers[2];
    glGenBuffers(2, buffers);
    m_vbo = buffers[0];
    m_cbo = buffers[1];

    // Create the vertex array object
    glGenVertexArrays(1, &m_vao);

    // Create the program
    if (!create_program(vertex_shader_source, fragment_shader_source, &m_program))
    {
        cgtub::log_message(cgtub::LogLevel::Error,
                           "PointRenderer::PointRenderer(): Failed to compile shaders. Points will not be rendered.");
    }
}

PointRenderer::PointRenderer(PointRenderer&& other) noexcept
    : m_window(other.m_window)
    , m_vbo(std::exchange(other.m_vbo, 0))
    , m_cbo(std::exchange(other.m_cbo, 0))
    , m_vao(std::exchange(other.m_vao, 0))
    , m_program(std::exchange(other.m_program, 0))
{
}

PointRenderer& PointRenderer::operator=(PointRenderer&& other) noexcept
{
    std::swap(this->m_window, other.m_window);
    std::swap(m_vbo, other.m_vbo);
    std::swap(m_cbo, other.m_cbo);
    std::swap(m_vao, other.m_vao);
    std::swap(m_program, other.m_program);

    return *this;
}

PointRenderer::~PointRenderer()
{
    // TODO: Delete OpenGL objects
}

void PointRenderer::update_buffers(std::span<float const> points, std::span<float const> colors)
{
    if (points.size() > m_capacity)
    {
        //
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, points.size_bytes(), points.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, colors.size_bytes(), colors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_capacity = points.size();
        m_size     = points.size();

        update_vertex_array_object();

        return;
    }

    // Update buffer data
    m_size = points.size();

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size_bytes(), points.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size_bytes(), colors.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointRenderer::update_vertex_array_object()
{
    glBindVertexArray(m_vao);

    // All three attributes get array data
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    // The point vertex buffer is layed out like [point_1, point_2, ...].
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // The color buffer is layed out as [color_1, color_2, ...].
    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // All attributes are *per instance*
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PointRenderer::render(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport)
{
    render(points, colors, view_matrix, projection_matrix, PointRenderParams{.size = 4.f}, viewport);
}

void PointRenderer::render(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, PointRenderParams const& params, std::optional<Rect> viewport)
{
    if (points.size() != colors.size())
    {
        log_message(LogLevel::Error, 
                    "PointRenderer::render(): The number of points (=%d) does not match the provided number of colors (=%d)", points.size(), colors.size());
        return;
    }

    std::span<float const> points_(reinterpret_cast<float const*>(points.data()), 3 * points.size());
    std::span<float const> colors_(reinterpret_cast<float const*>(colors.data()), 3 * colors.size());

    update_buffers(points_, colors_);

    gl_check_error(__FILE__, __LINE__);

    // Activate a viewport for rendering
    set_viewport(m_window, viewport);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(m_program);

    glUniformMatrix4fv(glGetUniformLocation(m_program, "view_matrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix));

    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection_matrix"),
                       1, GL_FALSE, glm::value_ptr(projection_matrix));


    glBindVertexArray(m_vao);

    int width  = 0;
    int height = 0;
    if (viewport)
    {
        width  = viewport->width;
        height = viewport->height;
    }
    else
    {
        glfwGetFramebufferSize(m_window, &width, &height);
    }

    glUniform1f(glGetUniformLocation(m_program, "two_over_width"),  2.f / static_cast<float>(width));
    glUniform1f(glGetUniformLocation(m_program, "two_over_height"), 2.f / static_cast<float>(height));

    glUniform1f(glGetUniformLocation(m_program, "point_size"), params.size);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(colors.size()));

    glBindVertexArray(0);

    gl_check_error(__FILE__, __LINE__);
}

} // namespace cgtub