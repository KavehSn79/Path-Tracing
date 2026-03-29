#pragma once

#include <optional>
#include <span>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "cgtub/primitives.hpp"

// Forward declarations
struct GLFWwindow;

namespace cgtub
{

struct PointRenderParams
{
    float size{1.f};
};

class PointRenderer
{
public:
    PointRenderer(GLFWwindow* window);

    PointRenderer(PointRenderer const&) = delete;

    PointRenderer(PointRenderer&& other) noexcept;
    
    PointRenderer& operator=(PointRenderer const&) = delete;

    PointRenderer& operator=(PointRenderer&& other) noexcept;

    virtual ~PointRenderer();


    void update_buffers(std::span<float const> points, std::span<float const> colors);

    void update_vertex_array_object();

    void render(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport = std::nullopt);

    void render(std::span<glm::vec3 const> points, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, PointRenderParams const& params, std::optional<Rect> viewport = std::nullopt);


private:
    GLFWwindow*            m_window;
    size_t                 m_size{0u};
    size_t                 m_capacity{0u};
    std::vector<glm::vec3> m_ctransfer;
    GLuint                 m_vbo{0u};
    GLuint                 m_cbo{0u};
    GLuint                 m_vao{0u};
    GLuint                 m_program{0u};
};

} // namespace cgtub