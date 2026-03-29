#pragma once

#include <optional>
#include <span>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cgtub/fwd.hpp>
#include <cgtub/line_renderer.hpp>

namespace cgtub
{

class NDCRenderer
{
public:
    NDCRenderer(Canvas& canvas);

    NDCRenderer(NDCRenderer const&) = delete;

    NDCRenderer(NDCRenderer&&) = delete;

    virtual ~NDCRenderer();

    NDCRenderer& operator=(NDCRenderer const&) = delete;

    NDCRenderer& operator=(NDCRenderer&&) = delete;

    /**
     * \brief Render a triangle mesh provided as index face set in Normalized Device Coordinates.
     *
     * The input coordinates should be unnormalized, i.e., for a point [x, y, z, w], the coordinate w should not necessarily be 1.
     * 
     * \param[in] positions Vertex positions of the mesh in Normalized Device Coordinates.
     * \param[in] indices   Indices that define the mesh faces (indexes into \p positions).
     * \param[in] color     Base color used to shade the mesh.
     * \param[in] id        Identifier assigned to the render call, used in conjunction with \c SimpleRenderer::clicked().
     *                      If negative, the render call has no id.
     */
    void render_mesh(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices, glm::vec3 const& color);

    /**
     * \brief Render a set of lines provided by start and end points in Normalized Device Coordinates.
     *
     * The input coordinates should be unnormalized, i.e., for a point [x, y, z, w], the coordinate w should not necessarily be 1.
     * 
     * \param[in] points Start/end points of the lines in Normalized Device Coordinates. Points are interleaved: [start_1, end_1, start_2, end_2, ...]
     * \param[in] colors A constant color for each line. The number of colors must match the number of lines, i.e., points.size() == 2 * colors.size().
     */
    void render_lines(std::span<glm::vec4 const> points, std::span<glm::vec3 const> colors);

private:
    void update_buffers(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices);
    void update_vertex_array_object(std::span<glm::vec4 const> positions, std::span<glm::u32vec3 const> indices);

    Canvas&      m_canvas;

    size_t m_size{0u};
    size_t m_capacity{0u};
    GLuint m_vbo{0u};
    GLuint m_ibo{0u};
    GLuint m_vao{0u};
    GLuint m_program{0u};

    LineRenderer           m_line_renderer;
    std::vector<glm::vec3> m_line_buffer;
};

} // namespace cgtub