#pragma once

#include <optional>
#include <span>

#include <glm/glm.hpp>

#include "cgtub/camera_perspective.hpp"
#include "cgtub/camera_controller_turntable.hpp"
#include "cgtub/fwd.hpp"
#include "cgtub/line_renderer.hpp"
#include "cgtub/mesh_renderer.hpp"
#include "cgtub/point_renderer.hpp"

namespace cgtub
{

/**
 * \brief A simple renderer for triangle meshes and lines.
 * 
 * A \c SimpleRenderer is always attached to a canvas, which is used as a render target.
 * 
 * It also manages a camera and its corresponding controller.
 */
class SimpleRenderer
{
public:
    // Construct a renderer that renders to the given canvas.
    SimpleRenderer(Canvas& canvas);

    SimpleRenderer(SimpleRenderer const&) = delete;

    SimpleRenderer(SimpleRenderer&&) = delete;

    SimpleRenderer& operator=(SimpleRenderer const&) = delete;

    SimpleRenderer& operator=(SimpleRenderer&&) = delete;

    virtual ~SimpleRenderer();


    // Handle events (resizing window, user input, etc.)
    void update(float dt, EventDispatcher* dispatcher);

    /**
     * \brief Render a triangle mesh provided as index face set.
     * 
     * \param[in] positions Vertex positions of the mesh in *world* space.
     * \param[in] indices   Indices that define the mesh faces (indexes into \p positions).
     * \param[in] color     Base color used to shade the mesh.
     * \param[in] id        Identifier assigned to the render call, used in conjunction with \c SimpleRenderer::clicked(). 
     *                      If negative, the render call has no id.
     */
    void render_mesh(std::span<glm::vec3 const> positions, std::span<glm::u32vec3 const> indices, glm::vec3 const& color, int id = -1);

    /**
     * \brief Render a set of lines provided by start and end points.
     * 
     * \param[in] points Start/end points of the lines in *world* space. Points are interleaved: [start_1, end_1, start_2, end_2, ...]
     * \param[in] colors A constant color for each line. The number of colors must match the number of lines, i.e., points.size() == 2 * colors.size().
     */
    void render_lines(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors);

    /**
     * \brief Render a set of points.
     *
     * \param[in] points Position of the points.
     * \param[in] colors A constant color for each point (by default, empty).
     */
    void render_points(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors);

    /**
     * \brief Query if a mesh has been clicked on and retrieve the mesh's id.
     * 
     * Each mesh render call can get assigned a unique \c id >= 0 using \c SimpleRenderer::render_mesh(..., id).
     * If a left mouse button click on a position in the canvas occurs, this function can identify the clicked-on mesh 
     * and return the id passed to the corresponding render call.
     * 
     * Implementation detail: a click on a mesh can only be registered if \c SimpleRenderer::update(...) is called
     *                        between the calls to \c SimpleRenderer::render_mesh(...) and \c SimpleRenderer::clicked(...).
     *                        Consequently, the clicks registered in the current frame are typically from the render calls
     *                        of the last frame.
     * 
     * \param[out] id Identifier of the render call associated with the mesh that was clicked on. 
     *                If there was a click, but not on a mesh, the parameter will be set to -1.
     *                If there was no click, \p id will not be modified.
     * 
     * \return True if a clicked occurs inside the canvas, false otherwise.
     */
    bool clicked(int* id) const;

    /**
     * \brief Indicator if the mouse is hovering over a mesh.
     * 
     * \param[out] id Identifier of the render call associated with the mesh the mouse is hovering over.
     *                If the mouse hovers inside the canvas, but not on a mesh, the parameter will be set to -1.
     *                If the mouse doesn't hover inside the canvas, \p id will not be modified.
     * 
     * \return True if the mouse is hovering inside the canvas, false otherwise.
     */
    bool hovered(int* id) const;


    // Get a handle to the camera managed by this renderer.
    PerspectiveCamera const& camera() const;

private:
    void resize_attachments();

    Canvas&                   m_canvas;
    MeshRenderer              m_mesh_renderer;
    LineRenderer              m_line_renderer;
    PointRenderer             m_point_renderer;
    PerspectiveCamera         m_camera;
    TurntableCameraController m_camera_controller;

    GLuint                   m_fbo;
    GLuint                   m_textures[2];
    int                      m_hover_id{0};
    int                      m_pick_id{0};
    std::optional<glm::vec2> m_pick_pos;
};

} // namespace cgtub