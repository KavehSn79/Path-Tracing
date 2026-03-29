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

struct LineRenderParams
{
    float width{1.f};
};

class LineRenderer
{
public:
    LineRenderer(GLFWwindow* window);

    LineRenderer(LineRenderer const&) = delete;

    LineRenderer(LineRenderer&& other) noexcept;

    LineRenderer& operator=(LineRenderer const&) = delete;

    LineRenderer& operator=(LineRenderer&& other) noexcept;

    virtual ~LineRenderer();


    void update_buffers(std::span<float const> vertices, std::span<float const> colors);

    void update_vertex_array_object();

    void render(std::span<glm::vec3 const> lines, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport = std::nullopt);

    void render(std::span<glm::vec3 const> lines, std::span<glm::vec3 const> colors, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, LineRenderParams const& params, std::optional<Rect> viewport = std::nullopt);


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