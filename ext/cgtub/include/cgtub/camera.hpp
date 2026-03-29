#pragma once

#include <glm/glm.hpp>

namespace cgtub
{

class Camera
{
public:
    Camera(float aspect, float z_near, float z_far);

    float aspect() const;

    float z_near() const;

    float z_far() const;

    glm::mat4 const& view() const;

    glm::mat4 const& inverse_view() const;

    virtual glm::mat4 const& projection() const = 0;

    virtual glm::mat4 const& inverse_projection() const;

    void set_aspect(float aspect);

    void set_z_near(float z_near);

    void set_z_far(float z_far);

    void set_view(glm::mat4 const& view);

protected:
    enum DirtyFlags : char
    {
        None              = 0b0000,
        Projection        = 0b0001,
        View              = 0b0010,
        InverseProjection = 0b0100,
        InverseView       = 0b1000,
    };

    float m_aspect;
    float m_z_near;
    float m_z_far;

    mutable char      m_dirty;
    mutable glm::mat4 m_view;
    mutable glm::mat4 m_projection;
    mutable glm::mat4 m_inverse_view;
    mutable glm::mat4 m_inverse_projection;
};

} // namespace cgtub