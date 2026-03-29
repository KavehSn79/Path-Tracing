#include <algorithm>
#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "cgtub/camera.hpp"
#include "cgtub/camera_controller_turntable.hpp"
#include "cgtub/canvas.hpp"
#include "cgtub/event_dispatcher.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

TurntableCameraController::TurntableCameraController(Canvas& canvas, Camera& camera)
    : CameraController(canvas, camera)
{
}

bool TurntableCameraController::auto_rotate() const
{
    return m_auto_rotate;
}

void TurntableCameraController::set_auto_rotate(bool auto_rotate)
{
    m_auto_rotate = auto_rotate;
}

void TurntableCameraController::update(float dt, EventDispatcher* dispatcher)
{
    CameraController::update(dt, dispatcher);
    InputEvents const& inputs = dispatcher->inputs();

    if (!m_enabled)
        return;

    if (m_auto_rotate)
        m_azimuth += m_speed * dt;

    // Handle input (do we have focus?)
    double xpos_d, ypos_d;
    glfwGetCursorPos(m_canvas.window(), &xpos_d, &ypos_d);
    float xpos = static_cast<float>(xpos_d);
    float ypos = static_cast<float>(ypos_d);

    bool is_inside_canvas = m_canvas.is_inside(static_cast<int>(xpos), 
                                               static_cast<int>(ypos));

    // Manage state changes between drag and idle
    if (inputs.button(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && is_inside_canvas)
    {
#if CGTUB_LEGACY_OUTPUTS
        log_message(LogLevel::Debug, "-> drag");
#endif
        m_state = InputState::Drag;
        m_xpos  = xpos;
        m_ypos  = ypos;
    }
    else if (inputs.button(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && m_state != InputState::Idle)
    {
#if CGTUB_LEGACY_OUTPUTS
        log_message(LogLevel::Debug, "-> idle");
#endif
        m_state = InputState::Idle;
    }

    // Handle dragging
    if (m_state == InputState::Drag)
    {
        float dxpos = xpos - m_xpos;
        float dypos = ypos - m_ypos;

        m_azimuth -= 0.01f * dxpos;

        m_elevation += 0.01f * dypos;

        static float const epsilon = 1e-4f;
        m_elevation                = std::clamp(m_elevation, -glm::half_pi<float>() + epsilon, glm::half_pi<float>() - epsilon);

        m_xpos = xpos;
        m_ypos = ypos;
    }

    // Handle zooming
    if (inputs.scroll.yoffset != 0 && is_inside_canvas)
    {
        m_distance -= 0.1f * inputs.scroll.yoffset;
    }

    // TODO: Clip azimuth and elevation range

    // Update the view matrix
    m_camera.set_view(build_view_matrix());
}

glm::mat4 TurntableCameraController::build_view_matrix() const
{
    // Build the view matrix
    glm::vec3 z = glm::vec3(std::cos(m_elevation) * std::sin(m_azimuth), std::sin(m_elevation), std::cos(m_elevation) * std::cos(m_azimuth));
    glm::vec3 x = glm::normalize(glm::vec3(std::cos(m_elevation) * std::cos(m_azimuth), 0.f, -std::cos(m_elevation) * std::sin(m_azimuth)));
    glm::vec3 y = glm::cross(z, x);

    glm::mat4 R = glm::transpose(glm::mat3(x, y, z));

    return R * glm::translate(glm::mat4(1), -m_distance * z);
}

} // namespace cgtub