#include <glm/gtc/matrix_transform.hpp>

#include "cgtub/camera.hpp"

namespace cgtub
{

Camera::Camera(float aspect, float z_near, float z_far)
    : m_aspect(aspect)
    , m_z_near(z_near)
    , m_z_far(z_far)
    , m_dirty(DirtyFlags::None)
    , m_view(glm::mat4(1))
    , m_projection(glm::mat4(1))
    , m_inverse_view(glm::mat4(1))
    , m_inverse_projection(glm::mat4(1))
{
}

float Camera::aspect() const
{
    return m_aspect;
}

float Camera::z_near() const
{
    return m_z_near;
}

float Camera::z_far() const
{
    return m_z_far;
}

glm::mat4 const& Camera::view() const
{
    return m_view;
}

glm::mat4 const& Camera::inverse_view() const
{
    if (m_dirty & DirtyFlags::InverseView)
    {
        m_inverse_view = glm::inverse(view());
        m_dirty        = m_dirty & ~DirtyFlags::InverseView;
    }

    return m_inverse_view;
}

glm::mat4 const& Camera::inverse_projection() const
{
    if (m_dirty & DirtyFlags::InverseProjection)
    {
        m_inverse_projection = glm::inverse(projection());
        m_dirty              = m_dirty & ~DirtyFlags::InverseProjection;
    }

    return m_inverse_projection;
}

void Camera::set_aspect(float aspect)
{
    m_aspect = aspect;
    m_dirty |= DirtyFlags::Projection;
    m_dirty |= DirtyFlags::InverseProjection;
}

void Camera::set_z_near(float z_near)
{
    m_z_near = z_near;
    m_dirty |= DirtyFlags::Projection;
    m_dirty |= DirtyFlags::InverseProjection;
}

void Camera::set_z_far(float z_far)
{
    m_z_far = z_far;
    m_dirty |= DirtyFlags::Projection;
    m_dirty |= DirtyFlags::InverseProjection;
}

void Camera::set_view(glm::mat4 const& view)
{
    m_view = view;
    m_dirty |= DirtyFlags::View;
    m_dirty |= DirtyFlags::InverseView;
}

} // namespace cgtub