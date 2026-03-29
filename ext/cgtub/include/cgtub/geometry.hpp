#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace cgtub
{

/**
 * \brief Generates the geometry of a box with non-uniform scaling, filling in vertex positions, indices, normals, and UV coordinates.
 *
 * \param[in] scale A 3D vector (glm::vec3) representing the box's scale along the x, y, and z axes.
 * \param[in, out] positions A pointer to a vector of glm::vec3 that will be filled with the box's vertex positions.
 * \param[in, out] indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the box's triangular faces.
 * \param[in, out] normals A pointer to a vector of glm::vec3 that will be filled with the normal vectors for each vertex.
 * \param[in, out] uvs A pointer to a vector of glm::vec2 that will be filled with the UV texture coordinates for each vertex.
 */
void create_box_geometry(glm::vec3 scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals = nullptr, std::vector<glm::vec2>* uvs = nullptr);

/**
 * \brief Generates the geometry of a box with uniform scale, filling in vertex positions and indices.
 *
 * \param[in] scale A scalar (float) representing uniform scaling for the box along all axes.
 * \param[in, out] positions A pointer to a vector of glm::vec3 that will be filled with the box's vertex positions.
 * \param[in, out] indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the box's triangular faces.
 */
void create_box_geometry(float scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices);

/**
 * \brief Generates the geometry of a sphere with a given resolution and non-uniform scaling, filling in vertex positions, triangle indices, normals, and UV coordinates.
 *
 * \param[in] n The number of segments along the longitudinal direction (latitude).
 * \param[in] m The number of segments along the latitudinal direction (longitude).
 * \param[in] scale A 3D vector (glm::vec3) representing the sphere's scale along the x, y, and z axes.
 * \param[in, out] positions A pointer to a vector of glm::vec3 that will be filled with the sphere's vertex positions.
 * \param[in, out] indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the sphere's triangular faces.
 * \param[in, out] normals A pointer to a vector of glm::vec3 that will be filled with the normal vectors for each vertex.
 * \param[in, out] uvs A pointer to a vector of glm::vec2 that will be filled with the UV texture coordinates for each vertex.
 */
void create_sphere_geometry(unsigned int n, unsigned int m, glm::vec3 scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals = nullptr, std::vector<glm::vec2>* uvs = nullptr);

/**
 * \brief Generates the geometry of a sphere with uniform scale, filling in vertex positions and triangle indices.
 *
 * \param[in] scale A scalar (float) representing uniform scaling for the sphere along all axes.
 * \param[in, out] positions A pointer to a vector of glm::vec3 that will be filled with the sphere's vertex positions.
 * \param[in, out] indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the sphere's triangular faces.
 */
void create_sphere_geometry(float scale, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices);

/**
 * \brief Generates the geometry of a torus with a given resolution and non-uniform scale, filling in vertex positions, triangle indices, normals, and UV coordinates.
 * 
 * \param n The number of segments along the longitudinal direction (latitude).
 * \param m The number of segments along the latitudinal direction (longitude).
 * \param r A 3D vector (glm::vec3) representing the torus' inner radius along the x, y, and z axes.
 * \param R A 3D vector (glm::vec3) representing the torus' outer radius along the x, y, and z axes.
 * \param positions A pointer to a vector of glm::vec3 that will be filled with the sphere's vertex positions.
 * \param indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the torus' triangular faces.
 * \param[in, out] normals A pointer to a vector of glm::vec3 that will be filled with the normal vectors for each vertex.
 * \param[in, out] uvs A pointer to a vector of glm::vec2 that will be filled with the UV texture coordinates for each vertex.
 */
void create_torus_geometry(unsigned int n, unsigned int m, glm::vec3 r, glm::vec3 R, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices, std::vector<glm::vec3>* normals = nullptr, std::vector<glm::vec2>* uvs = nullptr);

/**
 * \brief Generates the geometry of a torus with uniform scale, filling in vertex positions and triangle indices.
 *
 * \param r A scalar (float) representing uniform scaling for the inner radius along all axes.
 * \param R A scalar (float) representing uniform scaling for the outer radius along all axes.
 * \param positions A pointer to a vector of glm::vec3 that will be filled with the torus' vertex positions.
 * \param indices A pointer to a vector of glm::u32vec3 that will be filled with the indices of the torus' triangular faces.
 */
void create_torus_geometry(float r, float R, std::vector<glm::vec3>* positions, std::vector<glm::u32vec3>* indices);

} // namespace cgtub