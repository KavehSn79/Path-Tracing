#include <glm/gtc/matrix_transform.hpp>

#include "cgtub/camera_perspective.hpp"

namespace cgtub
{

PerspectiveCamera::PerspectiveCamera(float fov_y, float aspect, float z_near, float z_far)
    : Camera(aspect, z_near, z_far)
    , m_fov_y(fov_y)
{
}

float PerspectiveCamera::fov_y() const
{
    return m_fov_y;
}

glm::mat4 const& PerspectiveCamera::projection() const
{
    if (m_dirty & DirtyFlags::Projection)
    {
        m_projection = glm::perspectiveRH_NO(m_fov_y * glm::pi<float>() / 180.f, m_aspect, m_z_near, m_z_far);
        m_dirty      = m_dirty & ~DirtyFlags::Projection;
    }

    return m_projection;
}

void PerspectiveCamera::set_fov_y(float fov_y)
{
    m_fov_y = fov_y;
    m_dirty |= DirtyFlags::Projection;
    m_dirty |= DirtyFlags::InverseProjection;
}

} // namespace cgtub