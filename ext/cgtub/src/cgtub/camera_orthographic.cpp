#include <glm/gtc/matrix_transform.hpp>

#include "cgtub/camera_orthographic.hpp"

namespace cgtub
{

OrthographicCamera::OrthographicCamera(float size_y, float aspect, float z_near, float z_far)
    : Camera(aspect, z_near, z_far)
    , m_size_y(size_y)
{
}

float OrthographicCamera::size_y() const
{
    return m_size_y;
}

glm::mat4 const& OrthographicCamera::projection() const
{
    if (m_dirty & DirtyFlags::Projection)
    {
        // aspect = width/height
        float top    = m_size_y;
        float bottom = -m_size_y;
        float left   = -m_aspect * m_size_y;
        float right  = m_aspect * m_size_y;

        m_projection = glm::orthoRH_NO(left, right, bottom, top, m_z_near, m_z_far);
        m_dirty      = m_dirty & ~DirtyFlags::Projection;
    }

    return m_projection;
}

void OrthographicCamera::set_size_y(float size_y)
{
    m_size_y = size_y;
    m_dirty |= DirtyFlags::Projection;
    m_dirty |= DirtyFlags::InverseProjection;
}

} // namespace cgtub