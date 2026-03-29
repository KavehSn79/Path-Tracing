#include "cgtub/line_renderer.hpp"

#include <utility>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

#define CGTUB_INSTANCED_LINE_RENDERING 1

#if CGTUB_INSTANCED_LINE_RENDERING
constexpr const char* vertex_shader_source = R"glsl(
    #version 330

    uniform mat4 view_matrix;
    uniform mat4 projection_matrix;

    uniform float two_over_width;
    uniform float two_over_height;
    uniform float line_width;

    layout (location = 0) in vec3 start;
    layout (location = 1) in vec3 end;
    layout (location = 2) in vec3 color;

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

        uv.y -= 0.5f;

        // Transform the line start/end into clip space.
        // Could be done on the CPU but is it faster?
        vec4 start_ndc = projection_matrix * view_matrix * vec4(start, 1);
        vec4 end_ndc   = projection_matrix * view_matrix * vec4(end,   1);
        
        vec2 difference_2d = end_ndc.xy / end_ndc.w - start_ndc.xy / start_ndc.w;

        mat2 basis;
        basis[0] = normalize(difference_2d);
        basis[1] = vec2(-basis[0].y, basis[0].x);

        // uv.x is in normalized device coordinates (NDC), uv.y is in pixels
        uv.x *= length(difference_2d);
        uv.y *= line_width;

        // Since uv.y is in pixels, scale the basis vector so that the result is in NDC.
        uv = uv.x * basis[0] + uv.y * vec2(two_over_width * basis[1].x, two_over_height * basis[1].y);

        // Shift the quad to the line origin (start point)
        vec2 origin = start_ndc.xy / start_ndc.w;
        uv += origin;

        vec4 reference_ndc = (gl_VertexID & 1) == 0 ? start_ndc : end_ndc;

        gl_Position = vec4(uv.x * reference_ndc.w, uv.y * reference_ndc.w, reference_ndc.z, reference_ndc.w);
        
        vs_out.color = color;
    }
)glsl";
#else 
constexpr const char* vertex_shader_source = R"glsl(
    #version 330

    uniform mat4 view_matrix;
    uniform mat4 projection_matrix;

    in vec3 position;
    in vec3 color;

    out VertexData
    {
        vec3 color;
    }
    vs_out;

    void main()
    {
        vs_out.color = color;
        gl_Position = projection_matrix * view_matrix * vec4(position, 1);
    }
)glsl";
#endif
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


LineRenderer::LineRenderer(GLFWwindow* window)
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
                           "LineRenderer::LineRenderer(): Failed to compile shaders. Lines will not be rendered.");
    }
}

LineRenderer::LineRenderer(LineRenderer&& other) noexcept
    : m_window(other.m_window)
    , m_vbo(std::exchange(other.m_vbo, 0))
    , m_cbo(std::exchange(other.m_cbo, 0))
    , m_vao(std::exchange(other.m_vao, 0))
    , m_program(std::exchange(other.m_program, 0))
{
}

LineRenderer& LineRenderer::operator=(LineRenderer&& other) noexcept
{
    std::swap(this->m_window, other.m_window);
    std::swap(m_vbo, other.m_vbo);
    std::swap(m_cbo, other.m_cbo);
    std::swap(m_vao, other.m_vao);
    std::swap(m_program, other.m_program);

    return *this;
}

LineRenderer::~LineRenderer()
{
    // TODO: Delete OpenGL objects
}

void LineRenderer::update_buffers(std::span<float const> vertices, std::span<float const> colors)
{
    if (vertices.size() > m_capacity)
    {
        //
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, colors.size_bytes(), colors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_capacity = vertices.size();
        m_size     = vertices.size();

        update_vertex_array_object();

        return;
    }

    // Update buffer data
    m_size = vertices.size();

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size_bytes(), vertices.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size_bytes(), colors.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LineRenderer::update_vertex_array_object()
{
    glBindVertexArray(m_vao);

#if CGTUB_INSTANCED_LINE_RENDERING
    // All three attributes get array data
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    // The line vertex buffer is layed out like [start_1, end_1, start_2, end_2, ...].
    // Assign two attributes to the start and end (attention with the strides)
    GLsizei stride = 2 * 3 * sizeof(float);
    GLint64 offset = stride / 2;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);

    // The color buffer is layed out as [color_1, color_2, ...].
    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // All attributes are *per instance*
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
#else
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
#endif

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LineRenderer::render(std::span<glm::vec3 const> lines, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport)
{
    render(lines, colors, view_matrix, projection_matrix, {}, viewport);
}

void LineRenderer::render(std::span<glm::vec3 const> lines, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, LineRenderParams const& params, std::optional<Rect> viewport)
{
    if (lines.size() % 2 != 0)
    {
        log_message(LogLevel::Error,
                    "LineRenderer::render(): Lines are defined by start and end point but the size of input `lines` is not a multiple of two (size=%d)", lines.size());
        return;
    }

    if (lines.size() != 2 * colors.size())
    {
        log_message(LogLevel::Error, 
                    "LineRenderer::render(): The number of lines specified by start and end points (=%d) does not match the provided number of line colors (=%d)", lines.size() / 2, colors.size());
        return;
    }

    std::span<float const> vertices(reinterpret_cast<float const*>(lines.data()), 3 * lines.size());

#if CGTUB_INSTANCED_LINE_RENDERING
    std::span<float const> colors_(reinterpret_cast<float const*>(colors.data()), 3 * colors.size());
#else
    // Use a transfer buffer to interleave colors
    m_ctransfer.resize(2 * colors.size()); // 3 floats and two colors per line
    for (std::size_t i = 0; i < colors.size(); ++i)
    {
        glm::vec3 rgb          = colors[i];
        m_ctransfer[2 * i + 0] = rgb;
        m_ctransfer[2 * i + 1] = rgb;
    }
    std::span<float const> colors_(reinterpret_cast<float const*>(m_ctransfer.data()), 3 * m_ctransfer.size());
#endif

    update_buffers(vertices, colors_);

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

#if CGTUB_INSTANCED_LINE_RENDERING
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

    glUniform1f(glGetUniformLocation(m_program, "line_width"), params.width);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(colors.size()));
#else
    if (params.width != 1.f)
        log_message(LogLevel::Warn, "Wide lines are not supported by GL_LINES (requested width=%f)", params.width);

    size_t numVertices = m_size / 3; // 3 floats per vertex
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(numVertices));
#endif

    glBindVertexArray(0);

    gl_check_error(__FILE__, __LINE__);
}

} // namespace cgtub