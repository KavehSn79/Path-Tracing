#pragma once

#include <glm/glm.hpp>

#include "cgtub/camera.hpp"

namespace cgtub
{

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera(float size_y, float aspect, float z_near, float z_far);

    float size_y() const;

    glm::mat4 const& projection() const override;

    void set_size_y(float size_y);

private:
    float m_size_y;
};

} // namespace cgtub