#pragma once

#include <glm/glm.hpp>

#include "cgtub/fwd.hpp"

namespace cgtub
{

class CameraController
{
public:
    CameraController(Canvas& canvas, Camera& camera);

    void set_enabled(bool enabled);

    bool enabled() const;

    virtual void handle_resize();

    virtual void update(float dt, EventDispatcher* dispatcher);

protected:
    virtual void adapt_to_viewport(Rect const& viewport);

    Canvas& m_canvas;
    Camera& m_camera;
    bool    m_enabled;
};

} // namespace cgtub