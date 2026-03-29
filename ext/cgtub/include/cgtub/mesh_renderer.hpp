#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "cgtub/primitives.hpp"

// Forward declarations
struct GLFWwindow;

namespace cgtub
{

struct MeshRenderData
{
    glm::mat4                     matrix{glm::mat4(1)};
    std::span<const glm::vec3>    positions;
    std::span<const glm::vec3>    normals;
    std::span<const glm::vec3>    colors;
    std::span<const glm::u32vec3> indices;
    std::string_view              name{"unnamed"};
    unsigned int                  id{0u};
};

enum class MeshRenderMode
{
    Color,
    ColorLit,
    VertexColor,
    Position,
    Normal,
    Identifier,
    Unknown,
};

struct MeshRenderParams
{
    MeshRenderMode mode{MeshRenderMode::Position};
    glm::vec3      color{0.f, 1.f, 0.f};
};

class MeshRenderer
{
public:
    MeshRenderer(GLFWwindow* window);

    ~MeshRenderer();

    void update_buffers(MeshRenderData const& mesh);

    void update_vertex_array_object(MeshRenderData const& mesh, GLuint program);

    void render(MeshRenderData const& mesh, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, MeshRenderParams const& params, std::optional<Rect> viewport = std::nullopt);

    void render(MeshRenderData const& mesh, glm::mat4 const& view_matrix, glm::mat4 const& projection_matrix, std::optional<Rect> viewport = std::nullopt);

    GLFWwindow* m_window;

    size_t m_size{0u};
    size_t m_capacity{0u};
    GLuint m_vbo{0u};
    GLuint m_nbo{0u};
    GLuint m_cbo{0u};
    GLuint m_ibo{0u};
    GLuint m_vao{0u};
    struct
    {
        GLuint color{0u};
        GLuint colorlit{0u};
        GLuint vertexcolor{0u};
        GLuint position{0u};
        GLuint identifier{0u};
    } m_programs;
};

} // namespace cgtub