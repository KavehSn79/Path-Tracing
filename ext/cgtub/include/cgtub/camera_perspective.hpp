#pragma once

#include <glm/glm.hpp>

#include "cgtub/camera.hpp"

namespace cgtub
{

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(float fov_y, float aspect, float z_near, float z_far);

    float fov_y() const;

    glm::mat4 const& projection() const override;

    void set_fov_y(float fov_y);

private:
    float m_fov_y;
};

} // namespace cgtub