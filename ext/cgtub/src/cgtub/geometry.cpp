#include "cgtub/geometry.hpp"

#include <algorithm>
#include <numbers>
#include <tuple>
namespace cgtub
{

void create_box_geometry(glm::vec3 scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals, std::vector<glm::vec2>* uvs)
{
    // Clear all data
    positions->clear();
    indices->clear();
    if (normals)
        normals->clear();
    if (uvs)
        uvs->clear();

    // Preallocate memory
    positions->reserve(36);
    indices->reserve(12);
    if (normals)
        normals->reserve(36);
    if (uvs)
        uvs->reserve(36);

    // Helper function creating each triangulated quad of the cube individually
    auto create_plane = [&](glm::vec3 const& p, glm::vec3 const& u_dir, glm::vec3 const& v_dir)
    {
        size_t p_idx = positions->size();

        positions->emplace_back(scale * p);
        positions->emplace_back(scale * (p + u_dir));
        positions->emplace_back(scale * (p + u_dir + v_dir));
        positions->emplace_back(scale * p);
        positions->emplace_back(scale * (p + u_dir + v_dir));
        positions->emplace_back(scale * (p + v_dir));

        indices->emplace_back(p_idx, p_idx + 1, p_idx + 2);
        indices->emplace_back(p_idx + 3, p_idx + 4, p_idx + 5);

        if (normals)
        {
            glm::vec3 const normal = glm::normalize(glm::cross(u_dir, v_dir));
            for (int i = 0; i < 6; ++i)
                normals->emplace_back(normal);
        }

        if (uvs)
        {
            uvs->emplace_back(0, 1);
            uvs->emplace_back(1, 1);
            uvs->emplace_back(1, 0);
            uvs->emplace_back(0, 1);
            uvs->emplace_back(1, 0);
            uvs->emplace_back(0, 0);
        }
    };

    create_plane(glm::vec3(-1, -1, 1), glm::vec3(2, 0, 0), glm::vec3(0, 2, 0));
    create_plane(glm::vec3(1, -1, 1), glm::vec3(0, 0, -2), glm::vec3(0, 2, 0));
    create_plane(glm::vec3(1, -1, -1), glm::vec3(-2, 0, 0), glm::vec3(0, 2, 0));
    create_plane(glm::vec3(-1, -1, -1), glm::vec3(0, 0, 2), glm::vec3(0, 2, 0));
    create_plane(glm::vec3(-1, 1, 1), glm::vec3(2, 0, 0), glm::vec3(0, 0, -2));
    create_plane(glm::vec3(-1, -1, -1), glm::vec3(2, 0, 0), glm::vec3(0, 0, 2));
}

void create_box_geometry(float scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices)
{
    create_box_geometry(glm::vec3(scale), positions, indices);
}

void create_sphere_geometry(unsigned int n, unsigned int m, glm::vec3 scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals, std::vector<glm::vec2>* uvs)
{
    // Clear all data
    positions->clear();
    indices->clear();
    if (normals)
        normals->clear();
    if (uvs)
        uvs->clear();

    // Preallocate memory
    positions->reserve(n * m * 6);
    indices->reserve(2 * (n - 1) * m);
    if (normals)
        normals->reserve(n * m * 6);
    if (uvs)
        uvs->reserve(n * m * 6);

    // Helper function to compute vertex position on unit sphere
    auto compute_pos = [&](unsigned int i, unsigned int j) -> glm::vec3
    {
        float theta = static_cast<float>(i) / n * std::numbers::pi_v<float>;
        float phi   = static_cast<float>(j) / m * 2 * std::numbers::pi_v<float>;
        return glm::vec3(
            std::sin(theta) * std::sin(phi),
            std::cos(theta),
            std::sin(theta) * std::cos(phi));
    };

    // Helper function creating each triangulated quad of the sphere individually
    auto create_quad = [&](unsigned int i, unsigned int j)
    {
        size_t p_idx = positions->size();

        glm::vec3 v0 = compute_pos(i, j);
        glm::vec3 v1 = compute_pos(i + 1, j);
        glm::vec3 v2 = compute_pos(i + 1, (j + 1) % m);
        glm::vec3 v3 = compute_pos(i, (j + 1) % m);

        positions->emplace_back(scale * v0);
        positions->emplace_back(scale * v1);
        positions->emplace_back(scale * v2);
        positions->emplace_back(scale * v0);
        positions->emplace_back(scale * v2);
        positions->emplace_back(scale * v3);

        if (i < n - 1)
            indices->emplace_back(p_idx, p_idx + 1, p_idx + 2);
        if (i > 0)
            indices->emplace_back(p_idx + 3, p_idx + 4, p_idx + 5);

        if (normals)
        {
            glm::vec3 scale_inv(1.f / scale.x, 1.f / scale.y, 1.f / scale.z);
            normals->emplace_back(glm::normalize(scale_inv * v0));
            normals->emplace_back(glm::normalize(scale_inv * v1));
            normals->emplace_back(glm::normalize(scale_inv * v2));
            normals->emplace_back(glm::normalize(scale_inv * v0));
            normals->emplace_back(glm::normalize(scale_inv * v2));
            normals->emplace_back(glm::normalize(scale_inv * v3));
        }

        if (uvs)
        {
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i) / n);
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i) / n);
        }
    };

    for (unsigned int i = 0; i < n; i++)
    {
        for (unsigned int j = 0; j < m; j++)
        {
            create_quad(i, j);
        }
    }
}

void create_sphere_geometry(float scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices)
{
    create_sphere_geometry(16, 16, glm::vec3(scale), positions, indices);
}

void create_torus_geometry(unsigned int n, unsigned int m, glm::vec3 r, glm::vec3 R, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals, std::vector<glm::vec2>* uvs)
{
    // Clear all data
    positions->clear();
    indices->clear();
    if (normals)
        normals->clear();
    if (uvs)
        uvs->clear();

    // Preallocate memory
    positions->reserve(n * m * 6);
    indices->reserve(2 * (n - 1) * m);
    if (normals)
        normals->reserve(n * m * 6);
    if (uvs)
        uvs->reserve(n * m * 6);

    // Helper function to compute vertex position on unit torus
    auto compute_center_and_normal = [&](unsigned int i, unsigned int j) -> std::tuple<glm::vec3, glm::vec3>
    {
        float theta = static_cast<float>(i) / n * 2.f * std::numbers::pi_v<float>;
        float phi   = static_cast<float>(j) / m * 2.f * std::numbers::pi_v<float>;

        glm::vec3 center = glm::vec3(std::cos(phi), std::sin(phi), 0);
        glm::vec3 normal = glm::vec3(std::cos(theta) * std::cos(phi),
                                     std::cos(theta) * std::sin(phi),
                                     std::sin(theta));

        return std::make_tuple(center, normal);
    };

    // Normals have to be transformed with transpose(inverse(R))
    glm::vec3 R_inv(1.f / R.x, 1.f / R.y, 1.f / R.z);

    auto create_quad = [&](unsigned int i, unsigned int j)
    {
        size_t p_idx = positions->size();

        auto [c0, n0] = compute_center_and_normal(i, j);
        auto [c1, n1] = compute_center_and_normal(i + 1, j);
        auto [c2, n2] = compute_center_and_normal(i + 1, (j + 1) % m);
        auto [c3, n3] = compute_center_and_normal(i, (j + 1) % m);

        positions->emplace_back(R * c0 + r * glm::normalize(R_inv * n0));
        positions->emplace_back(R * c1 + r * glm::normalize(R_inv * n1));
        positions->emplace_back(R * c2 + r * glm::normalize(R_inv * n2));
        positions->emplace_back(R * c0 + r * glm::normalize(R_inv * n0));
        positions->emplace_back(R * c2 + r * glm::normalize(R_inv * n2));
        positions->emplace_back(R * c3 + r * glm::normalize(R_inv * n3));

        indices->emplace_back(p_idx, p_idx + 1, p_idx + 2);
        indices->emplace_back(p_idx + 3, p_idx + 4, p_idx + 5);

        if (normals)
        {
            normals->emplace_back(glm::normalize(R_inv * n0));
            normals->emplace_back(glm::normalize(R_inv * n1));
            normals->emplace_back(glm::normalize(R_inv * n2));
            normals->emplace_back(glm::normalize(R_inv * n0));
            normals->emplace_back(glm::normalize(R_inv * n2));
            normals->emplace_back(glm::normalize(R_inv * n3));
        }

        if (uvs)
        {
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i) / n);
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j) / m, static_cast<float>(i) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i + 1) / n);
            uvs->emplace_back(static_cast<float>(j + 1) / m, static_cast<float>(i) / n);
        }
    };

    for (unsigned int i = 0; i < n; i++)
    {
        for (unsigned int j = 0; j < m; j++)
        {
            create_quad(i, j);
        }
    }
}

void create_torus_geometry(float r, float R, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices)
{
    return create_torus_geometry(16, 16, glm::vec3(r), glm::vec3(R), positions, indices);
}

} // namespace cgtub
