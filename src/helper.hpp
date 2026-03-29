#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

// Forward declaration for SimpleRenderer
namespace cgtub
{
class SimpleRenderer;
class PerspectiveCamera;
}

namespace ex5
{

using GuiChanges = uint32_t;

enum class RenderMode
{
    Distance = 0,
    Position,
    Normal,
    MeshIndex,
    PrimitiveIndex,
    BRDFIndex,
    EmissionIndex,
    PathTracing,
};

/**
 * \brief Update the Graphical User Interface (GUI) and retrieve new values for the parameters.
 *
 * All non-const parameters are input/output, meaning their value will be used to display the GUI and
 * they will be set to the new value, as implied by user interaction with the GUI (in the previous frame).
 *
 * \return Object that tracks changes to the parameters.
 */
GuiChanges gui(int* subsampling_rate, RenderMode* render_mode, bool* do_render, bool* automatic_rendering, int* max_depth, int* samples_per_pixel, bool* accumulate_samples, bool* use_brdf_importance_sampling, bool* test_bounding_spheres, bool* show_bounding_spheres);

/**
 * \brief Query if an interaction with the GUI has changed a parameter value.
 *
 * Example usage:
 * \code{.cpp}
 * int   foo = ...;
 * float bar = ...;
 *
 * GuiChanges gui_changes = gui(&foo, &bar);
 *
 * if(has_gui_changed_parameter(gui_changes, 0))
 * {
 *     // Parameter `foo` was changed
 * }
 *
 * if(has_gui_changed_parameter(gui_changes, 1))
 * {
 *     // Parameter `bar` was changed
 * }
 * \endcode
 *
 * \param[in] changes         The \c GuiChanges returned by a call to \c gui(...)
 * \param[in] parameter_index The 0-based index of the parameter to query for changes
 */
bool has_gui_changed_parameter(GuiChanges gui_changes, uint32_t parameter_index);


struct Ray
{
    glm::vec3 o; //< The origin of the ray
    glm::vec3 d; //< The direction of the ray
};

struct Emission
{
    glm::vec3 radiance;

    std::vector<int> primitives;
};

enum class BRDFType
{
    Diffuse,
    Mirror
};

struct BRDF
{
    BRDFType type;

    struct Diffuse
    {
        glm::vec3 albedo;
    };

    struct Mirror
    {
    };

    union
    {
        Diffuse diffuse;
        Mirror  mirror;
    };
};

struct Mesh
{
    char const* name{""};

    std::vector<glm::vec3>    vertices;
    std::vector<glm::u32vec3> indices;

    int brdf_index{-1};     // if < 0, no brdf
    int emission_index{-1}; // if < 0, no emission
};

struct BoundingSphere
{
    glm::vec3 center{0.f};
    float     radius{0.f};
};

struct Scene
{
    glm::vec3 background{0.1, 0.1, 0.1};

    std::vector<Mesh>     meshes;
    std::vector<BRDF>     brdfs;
    std::vector<Emission> emissions;

    std::vector<BoundingSphere> bounding_volumes;
};

struct Intersection
{
    int mesh_index;
    int primitive_index;

    int brdf_index;
    int emission_index;

    float     t;      //< Distance from the ray origin to the intersection point
    glm::vec3 p;      //< World-space position of the intersection point
    glm::vec3 normal; //< World-space surface normal at the intersection point
};

void setup_scene(Scene* scene);

void rasterize_scene(cgtub::SimpleRenderer& renderer, Scene const& scene, bool show_bounding_spheres);

/**
 * \brief Generate a ray from the given camera at pixel (x,y) for an image with size (width, height)
 * 
 * \param[in] camera The perspective camera to generate a ray for.
 * \param[in] width  The width of the target image
 * \param[in] height The height of the target image
 * \param[in] x      x coordinate of the pixel position in [0, width-1]
 * \param[in] y      y coordinate of the pixel position in [0, height-1]
 */
ex5::Ray generate_ray(cgtub::PerspectiveCamera const& camera, int width, int height, float x, float y);

/*
 * \brief Generate a color for an intersection, depending on the render mode.
 * 
 * For example, if mode == RenderMode::Normal, then the color is generated from the normal `it.n`.
 * 
 * \param[in] mode Render mode that determines the returned color
 * \param[in] it   The intersection instance.
 */ 
glm::vec3 get_intersection_color(RenderMode mode, Intersection const& it);

/**
 * \brief Evaluate the emission E(x, w_o) from an intersection point it towards wo.
 * 
 * \param[in] scene The scene data structure
 * \param[in] it    The intersection instance.
 * \param[in] wo    The outgoing direction, pointing away from the emitter.
 * 
 * \return The emitted (RGB) radiance
 */
glm::vec3 eval_emission(Scene const& scene, ex5::Intersection const& it, glm::vec3 const& wo);

/**
 * \brief Sample a direction uniformly from the hemisphere at an intersection point.
 * 
 * \param[in]  it  The intersection instance 
 * \param[out] wi  The sampled direction in world space, pointing away from the intersection
 * \param[out] pdf The probability that the direction wi is sampled (constant 1/(2pi) for uniform sampling of the hemisphere)
 * 
 * \return Indicator if a valid direction was sampled
 */
bool sample_direction_uniform(Intersection const& it, glm::vec3* wi, float* pdf);

/**
 * \brief Evaluate the BRDF f(x,wo,wi) at an intersection point.
 *
 * \param[in]  scene The scene data structure
 * \param[in]  it    The intersection instance
 * \param[in]  wo    The outgoing direction in world space, pointing away from the intersection point
 * \param[in]  wi    The incoming direction in world space, pointing away from the intersection point
 *
 * \return The BRDF value for the RGB channels
 */
glm::vec3 eval_brdf(Scene const& scene, Intersection const& it, glm::vec3 const& wo, glm::vec3 const& wi);

// Get a uniformly distributed random value in [0, 1]
float get_uniform_random_value();

/**
 * \brief Construct a unit-norm frame (i.e., an orthonormal basis) from a given (unit-length) vector z
 * 
 * The returned vectors x and y fulfill the properties:
 * 1) ||x|| = ||y|| = 1
 * 2) <z,x> = 0, <z,y>=0, <x,y>=0
 * 3) det([x, y, z]) = 1
 *
 * \param[in]  z The scene data structure
 * \param[out] x The intersection instance
 * \param[out] y The outgoing direction in world space, pointing away from the intersection point
 */
void construct_frame(glm::vec3 const& z, glm::vec3* x, glm::vec3* y);

// Generate a unique, random color for an element with index `index`
glm::vec3 get_random_color(int index);

//================================================================================================================================
bool intersect(Scene const& scene, Ray const& ray, Intersection* it, bool test_bounding_spheres);


void render_intersection(Scene const& scene, cgtub::PerspectiveCamera const& camera,
                         int width, int height,
                         RenderMode              render_mode,
                         std::vector<glm::vec3>* image);

void render_pathtracing(Scene const& scene, cgtub::PerspectiveCamera const& camera,
                        int width, int height,
                        int                     max_depth,
                        int                     samples_per_pixel,
                        bool                    use_brdf_importance_sampling,
                        bool                    test_bounding_spheres,
                        std::vector<glm::vec3>* image);


bool sample_direction_from_brdf(ex5::Scene const& scene, ex5::Intersection const& it,
                                glm::vec3 const& wo, glm::vec3* wi, float* pdf);
} // namespace ex5