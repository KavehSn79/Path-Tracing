#include "cgtub/camera_controller.hpp"
#include "cgtub/camera.hpp"
#include "cgtub/canvas.hpp"
#include "cgtub/event_dispatcher.hpp"

namespace cgtub
{

CameraController::CameraController(Canvas& canvas, Camera& camera)
    : m_canvas(canvas)
    , m_camera(camera)
    , m_enabled(true)
{
    handle_resize();
}

void CameraController::set_enabled(bool enabled)
{
    m_enabled = enabled;
}

bool CameraController::enabled() const
{
    return m_enabled;
}

void CameraController::handle_resize()
{
    // Arguable if this is the responsibility of the controller
    adapt_to_viewport(m_canvas.viewport());
}

void CameraController::update(float dt, EventDispatcher* dispatcher)
{
    if (dispatcher->was_framebuffer_resized())
    {
        handle_resize();
    }
}

void CameraController::adapt_to_viewport(Rect const& viewport)
{
    if (viewport.width == 0 || viewport.height == 0)
        return;

    m_camera.set_aspect(viewport.width / static_cast<float>(viewport.height));
}

} // namespace cgtub