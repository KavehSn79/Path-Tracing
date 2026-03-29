#include <algorithm>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "cgtub/canvas.hpp"
#include "cgtub/event_dispatcher.hpp"
#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

Canvas::Canvas(GLFWwindow* window, Extent const& extent)
    : m_window(window)
    , m_extent(extent)
    , m_pixel_scaling{1.f, 1.f}
{
    if (!m_window)
        log_message(LogLevel::Error, "Canvas::Canvas(): 'window' is null. This will most certainly result in an unexpected crash.");

    if (extent.width < 0.f)
    {
        log_message(LogLevel::Warn, "Canvas::Canvas(): expected width > 0 but got 'extent.width'=%f. Clamping to 0 (canvas will not be visible).", extent.width);
        m_extent.width = 0.f;
    }

    if (extent.height < 0.f)
    {
        log_message(LogLevel::Warn, "Canvas::Canvas(): expected height > 0 but got 'extent.height'=%f. Clamping to 0 (canvas will not be visible).", extent.height);
        m_extent.height = 0.f;
    }

    handle_resize();
}

void Canvas::update(float dt, EventDispatcher const* dispatcher)
{
    if (dispatcher->was_framebuffer_resized())
    {
        handle_resize();
    }
}

GLFWwindow* Canvas::window()
{
    return m_window;
}

Rect Canvas::viewport(bool return_size_on_window) const
{
    if (!return_size_on_window)
        return m_viewport;
    else
        return Rect{
            .x      = static_cast<int>(m_viewport.x / m_pixel_scaling[0]),
            .y      = static_cast<int>(m_viewport.y / m_pixel_scaling[1]),
            .width  = static_cast<int>(m_viewport.width / m_pixel_scaling[0]),
            .height = static_cast<int>(m_viewport.height / m_pixel_scaling[1])};
}

void Canvas::clear(glm::vec3 const& color)
{
    ::cgtub::clear(m_window, color.r, color.g, color.b, 1.f, viewport()); // Clear the canvas with a different color
}

void Canvas::map_to_canvas(int* x, int* y) const
{
    *x = *x - m_viewport.x / m_pixel_scaling[0];
    *y = *y - m_viewport.y / m_pixel_scaling[1];
}

bool Canvas::is_inside(int x, int y) const
{
    // Transform coordinates from window space to framebuffer space by applying the scale.
    x *= m_pixel_scaling[0];
    y *= m_pixel_scaling[1];

    bool is_inside_x = (x >= m_viewport.x && x <= m_viewport.x + m_viewport.width);
    bool is_inside_y = (y >= m_viewport.y && y <= m_viewport.y + m_viewport.height);

    return is_inside_x && is_inside_y;
}

void Canvas::handle_resize()
{
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);

    m_viewport = Rect{
        .x      = static_cast<int>(std::floor(width * m_extent.x)),
        .y      = static_cast<int>(std::floor(height * m_extent.y)),
        .width  = static_cast<int>(std::ceil(width * m_extent.width)),
        .height = static_cast<int>(std::ceil(height * m_extent.height))};

    // Window size is not necessarily framebuffer size (e.g. on MacOS with pixel scaling).
    int window_width  = 0;
    int window_height = 0;
    glfwGetWindowSize(m_window, &window_width, &window_height);
    m_pixel_scaling[0] = static_cast<float>(width) / window_width;
    m_pixel_scaling[1] = static_cast<float>(height) / window_height;
}

} // namespace cgtub