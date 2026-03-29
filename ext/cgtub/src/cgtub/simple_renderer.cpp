#include "cgtub/simple_renderer.hpp"

#include "GLFW/glfw3.h"

#include "cgtub/canvas.hpp"
#include "cgtub/event_dispatcher.hpp"
#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

SimpleRenderer::SimpleRenderer(Canvas& canvas)
    : m_canvas(canvas)
    , m_mesh_renderer(canvas.window())
    , m_line_renderer(canvas.window())
    , m_point_renderer(canvas.window())
    , m_camera(45.f, 1, 0.01f, 10.f)
    , m_camera_controller(canvas, m_camera)
{
    // Use a custom framebuffer object for picking
    glGenFramebuffers(1, &m_fbo);

    // Two textures are needed: one for the id (color attachment) and one for the depth (depth attachment)
    glGenTextures(2, m_textures);
    resize_attachments();
}

SimpleRenderer::~SimpleRenderer()
{
    if (m_textures[0] != 0)
        glDeleteTextures(2, m_textures);
    if (m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
}

void SimpleRenderer::update(float dt, EventDispatcher* dispatcher)
{
    m_camera_controller.update(dt, dispatcher);

    // Handle window resize events
    if (dispatcher->was_framebuffer_resized())
    {
        resize_attachments();
    }

    // TODO: Lazily enable picking support (only if requested)

    // If the canvas size is 0 (e.g. if the window is minimized) there's nothing to do.
    // This also avoids an OpenGL error when clearing `m_fbo` with size (0, 0).
    Rect viewport = m_canvas.viewport();
    if (viewport.width == 0 || viewport.height == 0)
        return;

    double xpos_d, ypos_d;
    glfwGetCursorPos(m_canvas.window(), &xpos_d, &ypos_d);
    float xpos = static_cast<float>(xpos_d);
    float ypos = static_cast<float>(ypos_d);

    bool is_inside_canvas = m_canvas.is_inside(static_cast<int>(xpos), static_cast<int>(ypos));

    // Read from pick buffer and clear it for the next frame
    float pixel[4] = {-2, -2, -2, -2};
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    if (is_inside_canvas)
    {
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(m_canvas.window(), &width, &height);

        // Window size is not necessarily framebuffer size (e.g. on MacOS with pixel scaling).
        // Transform coordinates from window space to framebuffer space by applying the scale.
        // TODO: Could precompute the scaling on resize.
        int window_width  = 0;
        int window_height = 0;
        glfwGetWindowSize(m_canvas.window(), &window_width, &window_height);
        float scaling_x = static_cast<float>(width) / window_width;
        float scaling_y = static_cast<float>(height) / window_height;

        glReadPixels(static_cast<int>(scaling_x * xpos), height - static_cast<int>(scaling_y * ypos), 1, 1, GL_RGBA, GL_FLOAT, pixel);
    }
    ::cgtub::clear(m_canvas.window(), -1.f, -1.f, -1.f, -1.f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set the hovering id in any case and reset the pick id
    m_hover_id = static_cast<int>(pixel[0]);
    m_pick_id  = -2;

    // If a user clicks (left mouse) inside the canvas, store the currently clicked position (`m_pick_pos`)
    InputEvents const& inputs = dispatcher->inputs();
    if (inputs.button(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && is_inside_canvas)
    {
        m_pick_pos = glm::vec2(xpos, ypos);
    }

    // If the user moves the mouse too far from the pick position
    // (within some tolerance), picking is not registered
    if (m_pick_pos)
    {
        double dx = xpos - m_pick_pos->x;
        double dy = ypos - m_pick_pos->y;

        if ((dx * dx + dy * dy) > 8.0) 
            m_pick_pos = std::nullopt;
    }

    // Register the pick if the left mouse button was released
    if (m_pick_pos && inputs.button(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
    {
        m_pick_id  = m_hover_id;
        m_pick_pos = std::nullopt;
    }
}

void SimpleRenderer::render_mesh(std::span<glm::vec3 const> positions, std::span<glm::u32vec3 const> indices, glm::vec3 const& color, int id)
{
    // Catch and emit warnings here with very user friendly messages before the lower level renderer emits its (possibly uninformative) warnings

    if (positions.empty())
    {
        log_message(LogLevel::Warn, "SimpleRenderer::render_mesh(): Mesh does not have vertex positions. Did you forget to populate an array?");
        return;
    }

    if (indices.empty())
    {
        log_message(LogLevel::Warn, "SimpleRenderer::render_mesh(): Mesh does not have indices. Did you forget to populate an array?");
        return;
    }

    if (id < 0)
    {
        log_message(LogLevel::Trace, "SimpleRenderer::renderMesh(): id is negative (mesh is ignored for picking)");
    }

    // If the canvas size is 0 (e.g. if the window is minimized) there's nothing to do.
    // This also avoids an OpenGL error when rendering to `m_fbo` with size (0, 0).
    Rect viewport = m_canvas.viewport();
    if (viewport.width == 0 || viewport.height == 0)
    {
        log_message(LogLevel::Trace, "SimpleRenderer::renderMesh(): canvas has size 0, nothing is rendererd");
        return;
    }

    MeshRenderData meshData{
        .positions = positions,
        .indices   = indices,
        .id        = static_cast<unsigned int>(std::max(id, 0))};

    // 1st pass: render id to picking framebuffer (optional)
    if (id >= 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        m_mesh_renderer.render(meshData, m_camera.view(), m_camera.projection(), MeshRenderParams{.mode = MeshRenderMode::Identifier}, m_canvas.viewport());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2nd pass: render color to default framebuffer
    m_mesh_renderer.render(meshData, m_camera.view(), m_camera.projection(), MeshRenderParams{.mode = MeshRenderMode::ColorLit, .color = color}, m_canvas.viewport());
}

void SimpleRenderer::render_lines(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors)
{
    if (points.size() % 2 != 0)
    {
        log_message(LogLevel::Error,
                    "SimpleRenderer::render_lines(): Lines are defined by start and end point but \
                     the size of input `lines` is not a multiple of two (size=%d)",
                    points.size());
        return;
    }

    if (points.size() != 2 * colors.size())
    {
        log_message(LogLevel::Error,
                    "SimpleRenderer::render_lines(): The number of lines specified by start and end \
                     points (=%d) does not match the provided number of line colors (=%d)",
                    points.size() / 2, colors.size());
        return;
    }

    if (points.empty())
    {
        log_message(LogLevel::Warn, "SimpleRenderer::render_lines(): No lines provided. Did you forget to populate an array?");
        return;
    }

    m_line_renderer.render(points, colors, m_camera.view(), m_camera.projection(), m_canvas.viewport());
}

void SimpleRenderer::render_points(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors)
{
    if (points.empty())
    {
        log_message(LogLevel::Warn, "SimpleRenderer::render_points(): No points provided. Did you forget to populate an array?");
        return;
    }

    if (points.size() != colors.size())
    {
        log_message(LogLevel::Error,
                    "SimpleRenderer::render_points(): The number of points (=%d) \
                    does not match the provided number of colors (=%d)",
                    points.size(), colors.size());
        return;
    }

    m_point_renderer.render(points, colors, m_camera.view(), m_camera.projection(), m_canvas.viewport());
}

bool SimpleRenderer::hovered(int* id) const
{
    if (m_hover_id < -1)
        return false;

    if (id)
        *id = m_hover_id;

    return true;
}

bool SimpleRenderer::clicked(int* id) const
{
    if (m_pick_id < -1)
        return false;

    if (id)
        *id = m_pick_id;

    return true;
}

PerspectiveCamera const& SimpleRenderer::camera() const
{
    return m_camera;
}

void SimpleRenderer::resize_attachments()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(m_canvas.window(), &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Add/resize the color attachment
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[0], 0);

    // Add/resize the depth attachment
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_textures[1], 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


} // namespace cgtub
