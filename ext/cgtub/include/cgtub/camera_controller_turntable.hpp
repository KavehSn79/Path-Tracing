#pragma once

#include "cgtub/camera_controller.hpp"

namespace cgtub
{

class TurntableCameraController : public CameraController
{
public:
    TurntableCameraController(Canvas& canvas, Camera& camera);

    bool auto_rotate() const;

    void set_auto_rotate(bool auto_rotate);

    void update(float dt, EventDispatcher* dispatcher) override;

private:
    glm::mat4 build_view_matrix() const;

    enum InputState
    {
        Idle,
        Drag
    };

    bool  m_auto_rotate{false};
    float m_speed{1.5f};
    float m_azimuth{0.0f};
    float m_elevation{0.3f};
    float m_distance{2.f};

    InputState m_state{InputState::Idle};
    float      m_xpos{0.0};
    float      m_ypos{0.0};
};

} // namespace cgtub