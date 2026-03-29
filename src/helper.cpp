#include "helper.hpp"

#include <complex>
#include <random>
#include <algorithm>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include <cgtub/geometry.hpp>
#include <cgtub/camera_perspective.hpp>
#include <cgtub/log.hpp>
#include <cgtub/simple_renderer.hpp>

namespace ex5
{

GuiChanges gui(int* subsampling_rate, RenderMode* render_mode, bool* do_render, bool* automatic_rendering, int* max_depth, int* samples_per_pixel, bool* accumulate_samples, bool* use_brdf_importance_sampling, bool* test_bounding_spheres, bool* show_bounding_spheres)
{
    GuiChanges changes{0};

    ImGui::Begin("Exercise 5");

    int param_idx = 0;

    if (ImGui::SliderInt("Subsampling Rate", subsampling_rate, 1, 64))
        changes |= 1 << 0;

    if (ImGui::Combo("Render Mode", reinterpret_cast<int*>(render_mode), "Distance\0Position\0Normal\0Mesh Index\0Primitive Index\0BRDF Index\0Emitter Index\0Path Tracing\0"))
        changes |= 1 << 1;

    if (*do_render = (ImGui::Button("Render")))
        changes |= 1 << 2;

    ImGui::SameLine();
    if (ImGui::Checkbox("Automatic Rendering", automatic_rendering))
        changes |= 1 << 3;

    if (ImGui::SliderInt("Max. Depth", max_depth, 1, 16))
        changes |= 1 << 4;

    if (ImGui::SliderInt("Samples per Pixel", samples_per_pixel, 1, 64))
        changes |= 1 << 5;

    if (ImGui::Checkbox("Accumulate Samples", accumulate_samples))
        changes |= 1 << 6;

    if (ImGui::Checkbox("Use BRDF Importance Sampling", use_brdf_importance_sampling))
        changes |= 1 << 7;

    if (ImGui::Checkbox("Test Bounding Spheres", test_bounding_spheres))
        changes |= 1 << 8;

    if (ImGui::Checkbox("Show Bounding Spheres", show_bounding_spheres))
        changes |= 1 << 9;

    ImGuiIO& io = ImGui::GetIO();
    // TODO: Report FPS with 2 decimal precision
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();

    return changes;
}

bool has_gui_changed_parameter(GuiChanges gui_changes, uint32_t parameter_index)
{
    if (parameter_index >= 32)
        return false;

    return static_cast<bool>(gui_changes & (1 << parameter_index));
}

std::vector<glm::vec3> transform_affine(glm::mat4 const& mat, std::vector<glm::vec3> const& positions)
{
    std::vector<glm::vec3> positions_transformed = positions;
    for (glm::vec3& p : positions_transformed)
    {
        p = mat * glm::vec4(p, 1);
    }
    return positions_transformed;
}

std::vector<glm::vec3> transform_affine(glm::vec3 const& scale, glm::vec3 const& translation, glm::vec4 const& rotation_axis_angle, std::vector<glm::vec3> const& positions)
{
    glm::mat4 S(1);
    glm::mat4 T(1);
    for (int i = 0; i < 3; ++i)
    {
        S[i][i] = scale[i];
        T[3][i] = translation[i];
    }

    float     angle = rotation_axis_angle.w;
    glm::mat4 R     = angle != 0 ? glm::rotate(glm::mat4(1), angle, glm::normalize(glm::vec3(rotation_axis_angle))) : glm::mat4(1);

    return transform_affine(T * R * S, positions);
}

void setup_scene(Scene* scene)
{
    scene->background = glm::vec3(0.2, 0.2, 0.2);
    scene->brdfs.push_back(BRDF{.type = BRDFType::Diffuse, .diffuse = {.albedo = {0.570068, 0.0430135, 0.0443706}}});
    scene->brdfs.push_back(BRDF{.type = BRDFType::Diffuse, .diffuse = {.albedo = {0.105421, 0.37798, 0.076425}}});
    scene->brdfs.push_back(BRDF{.type = BRDFType::Diffuse, .diffuse = {.albedo = {0.885809, 0.698859, 0.666422}}});
    scene->brdfs.push_back(BRDF{.type = BRDFType::Mirror});
    scene->brdfs.push_back(BRDF{.type = BRDFType::Diffuse, .diffuse = {.albedo = {1.f, 1.f, 1.f}}});
    glm::vec3 emitter_color     = glm::vec3(1.f, 0.76072f, 0.36730f); // glm::vec3(18.387, 13.9873, 6.75357);
    float     emitter_intensity = 7.5f;
    scene->emissions.push_back(Emission{.radiance = emitter_intensity * emitter_color, .primitives = {10, 11}});

    // Create the
    std::vector<glm::vec3>    box_positions;
    std::vector<glm::u32vec3> box_indices;
    cgtub::create_box_geometry(1, &box_positions, &box_indices);

    std::vector<glm::vec3>    sphere_positions;
    std::vector<glm::u32vec3> sphere_indices;
    cgtub::create_sphere_geometry(16, 16, glm::vec3(1), &sphere_positions, &sphere_indices);

    float scale = 0.6f;
    scene->meshes.push_back(Mesh{.name = "left", .vertices = transform_affine(glm::vec3(0.02, scale, scale), glm::vec3(-scale, 0, 0), glm::vec4(0), box_positions), .indices = box_indices, .brdf_index = 0});
    scene->meshes.push_back(Mesh{.name = "right", .vertices = transform_affine(glm::vec3(0.02, scale, scale), glm::vec3(scale, 0, 0), glm::vec4(0), box_positions), .indices = box_indices, .brdf_index = 1});
    scene->meshes.push_back(Mesh{.name = "back", .vertices = transform_affine(glm::vec3(scale, scale, 0.02), glm::vec3(0, 0, -scale), glm::vec4(0), box_positions), .indices = box_indices, .brdf_index = 2});
    scene->meshes.push_back(Mesh{.name = "floor", .vertices = transform_affine(glm::vec3(scale, 0.02, scale), glm::vec3(0, -scale, 0), glm::vec4(0), box_positions), .indices = box_indices, .brdf_index = 2});
    scene->meshes.push_back(Mesh{.name = "top", .vertices = transform_affine(glm::vec3(scale, 0.02, scale), glm::vec3(0, scale, 0), glm::vec4(0), box_positions), .indices = box_indices, .brdf_index = 2});
    scene->meshes.push_back(Mesh{.name = "light", .vertices = transform_affine(glm::vec3(0.4, 0.02, 0.4), glm::vec3(0, scale - 0.01, 0), glm::vec4(0), box_positions), .indices = box_indices, .emission_index = 0});
    
    scene->meshes.push_back(Mesh{.name = "box", .vertices = transform_affine(glm::vec3(0.15, 0.3, 0.15), glm::vec3(0.25, -0.28, 0.15), glm::vec4(0, 1, 0, glm::quarter_pi<float>()), box_positions), .indices = box_indices, .brdf_index = 4});
    scene->meshes.push_back(Mesh{.name = "ball", .vertices = transform_affine(glm::vec3(0.25), glm::vec3(-0.35, -0.34, -0.35), glm::vec4(0), sphere_positions), .indices = sphere_indices, .brdf_index = 3});

    // Build bounding volumes for the shapes
    for (size_t mesh_idx = 0; mesh_idx < scene->meshes.size(); ++mesh_idx)
    {
        ex5::Mesh const& mesh = scene->meshes[mesh_idx];

        ex5::BoundingSphere bsphere{.center = glm::vec3(0), .radius = -std::numeric_limits<float>::max()};

        for (size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            bsphere.center += mesh.vertices[i];
        }
        bsphere.center /= static_cast<float>(mesh.vertices.size());

        for (size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            glm::vec3 v_centered = mesh.vertices[i] - bsphere.center;
            bsphere.radius       = std::max(bsphere.radius, glm::length(v_centered));
        }

        scene->bounding_volumes.push_back(bsphere);
    }
}

void render_sphere(cgtub::SimpleRenderer& renderer, glm::vec3 center, float radius)
{
    static std::vector<glm::vec3>    vertices;
    static std::vector<glm::u32vec3> indices;
    static std::vector<glm::vec3>    wireframe_line_points;
    static std::vector<glm::vec3>    wireframe_line_colors;
    static std::vector<glm::vec3>    wireframe_line_points_transformed;

    if (vertices.empty())
    {
        // Generate lines for the sphere
        cgtub::create_sphere_geometry(16, 16, glm::vec3(1.f), &vertices, &indices);

        for (glm::u32vec3 const& face : indices)
        {
            wireframe_line_points.push_back(vertices[face[0]]);
            wireframe_line_points.push_back(vertices[face[1]]);
            wireframe_line_points.push_back(vertices[face[1]]);
            wireframe_line_points.push_back(vertices[face[2]]);
            wireframe_line_points.push_back(vertices[face[2]]);
            wireframe_line_points.push_back(vertices[face[0]]);

            wireframe_line_colors.push_back(glm::vec3(1, 0, 0));
            wireframe_line_colors.push_back(glm::vec3(1, 0, 0));
            wireframe_line_colors.push_back(glm::vec3(1, 0, 0));
        }

        wireframe_line_points_transformed.resize(wireframe_line_points.size());
    }

    for (size_t i = 0; i < wireframe_line_points.size(); ++i)
        wireframe_line_points_transformed[i] = (wireframe_line_points[i] * radius) + center;

    renderer.render_lines(wireframe_line_points_transformed, wireframe_line_colors);
}

void rasterize_scene(cgtub::SimpleRenderer& renderer, Scene const& scene, bool show_bounding_spheres)
{
    // Render the shapes
    for (Mesh const& mesh : scene.meshes)
    {
        glm::vec3 color(1);
        if (mesh.emission_index >= 0)
        {
            Emission const& emitter = scene.emissions[mesh.emission_index];
            color                        = emitter.radiance;
        }
        else if (mesh.brdf_index >= 0)
        {
            BRDF const& brdf = scene.brdfs[mesh.brdf_index];
            if (brdf.type == BRDFType::Diffuse)
                color = brdf.diffuse.albedo;
        }

        renderer.render_mesh(mesh.vertices, mesh.indices, color);
    }

    // Render the bounding volumes
    if (show_bounding_spheres)
    {
        for (BoundingSphere const& bsphere : scene.bounding_volumes)
        {
            render_sphere(renderer, bsphere.center, bsphere.radius);
        }
    }
}

ex5::Ray generate_ray(cgtub::PerspectiveCamera const& camera, int width, int height, float x, float y)
{
    glm::mat4 inverse_view = camera.inverse_view();
    glm::mat4 inverse_proj = camera.inverse_projection();

    float x_ndc = 2 * ((x + 0.5f) / width) - 1;
    float y_ndc = 2 * ((y + 0.5f) / height) - 1;

    // Transform the pixel from NDC to view space and get the direction in view space
    glm::vec4 px_view  = inverse_proj * glm::vec4(x_ndc, y_ndc, 0, 1);
    glm::vec3 dir_view = glm::vec3(px_view) / px_view.w;

    // Transform the direction to world space
    glm::vec3 dir_world = glm::normalize(inverse_view * glm::vec4(dir_view, 0));

    return Ray{.o = inverse_view[3], .d = dir_world};
}

float get_uniform_random_value()
{
    static std::default_random_engine            engine(std::random_device{}());
    static std::uniform_real_distribution<float> distribution;
    return distribution(engine);
}

glm::vec3 get_intersection_color(RenderMode mode, Intersection const& it)
{
    if (mode == ex5::RenderMode::Distance)
        return glm::vec3(1 - 0.25f * it.t);
    if (mode == ex5::RenderMode::Position)
        return 0.5f * (it.p + 1.f);
    if (mode == ex5::RenderMode::Normal)
        return 0.5f * (it.normal + 1.f);
    if (mode == ex5::RenderMode::MeshIndex)
        return ex5::get_random_color(it.mesh_index);
    if (mode == ex5::RenderMode::PrimitiveIndex)
        return ex5::get_random_color(it.primitive_index);
    if (mode == ex5::RenderMode::BRDFIndex)
        return ex5::get_random_color(it.brdf_index);
    if (mode == ex5::RenderMode::EmissionIndex)
        return ex5::get_random_color(it.emission_index);

    return glm::vec3(0);
}

glm::vec3 eval_emission(Scene const& scene, Intersection const& it, glm::vec3 const& wo)
{
    // No emitter -> no output
    if (it.emission_index < 0)
        return glm::vec3(0);

    if (it.emission_index > scene.emissions.size())
    {
        cgtub::log_message(cgtub::LogLevel::Error, "ex5::eval_emission(): `emission_index` is %d, which is not a valid index for scene.emissions with size %d", it.emission_index, scene.emissions.size());
        return glm::vec3(0);
    }

    Emission const& emission = scene.emissions[it.emission_index];

    if (!emission.primitives.empty())
    {
        if (emission.primitives.end() == std::find(emission.primitives.begin(), emission.primitives.end(), it.primitive_index))
            return glm::vec3(0);
    }

    return scene.emissions[it.emission_index].radiance;
}

bool sample_direction_uniform(Intersection const& it, glm::vec3* wi, float* pdf)
{
    float sample1 = ex5::get_uniform_random_value();
    float sample2 = ex5::get_uniform_random_value();


    // Uniform sampling of the sphere
    float theta = std::acos(sample1);
    float phi   = 2 * glm::pi<float>() * sample2;

    *wi  = glm::vec3(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
    *pdf = glm::one_over_two_pi<float>();

    // Convert wi from local surface space to world space
    glm::vec3 x;
    glm::vec3 y;
    ex5::construct_frame(it.normal, &x, &y);
    *wi  = wi->x * x + wi->y * y + wi->z * it.normal;

    return true;
}

glm::vec3 eval_brdf(Scene const& scene, Intersection const& it, glm::vec3 const& wo, glm::vec3 const& wi)
{
    // No BRDF -> no light reflected
    if (it.brdf_index < 0)
        return glm::vec3(0);

    if (it.brdf_index > scene.brdfs.size())
    {
        cgtub::log_message(cgtub::LogLevel::Error, "ex5::eval_brdf(): `brdf_index` is %d, which is not a valid index for scene.brdfs with size %d", it.brdf_index, scene.brdfs.size());
        return glm::vec3(0);
    }

    ex5::BRDF const& brdf = scene.brdfs[it.brdf_index];

    if (brdf.type == ex5::BRDFType::Diffuse)
        return brdf.diffuse.albedo / glm::pi<float>();
    else if (brdf.type == ex5::BRDFType::Mirror)
        return glm::vec3(1);

    return glm::vec3(0);
}

glm::vec3 get_random_color(int index)
{
    static std::default_random_engine            engine(0);
    static std::uniform_real_distribution<float> distribution;
    static std::vector<glm::vec3>                random_colors;
    // Well, "unique" if the number of triangles is below 1024 :)
    constexpr int                                num_random_colors = 1024;

    // Fill the color array on the first call
    if (random_colors.empty())
    {
        random_colors.resize(num_random_colors);
        for (size_t i = 0; i < random_colors.size(); ++i)
        {
            random_colors[i] = glm::vec3(distribution(engine), distribution(engine), distribution(engine));
        }
    }

    return random_colors[std::abs(index) % num_random_colors];
}

void construct_frame(glm::vec3 const& z, glm::vec3* x, glm::vec3* y)
{
    *x = glm::normalize(glm::cross(z + glm::vec3(0.1, 0.2, 0.3), z));
    *y = glm::normalize(glm::cross(z, *x));
}



// ============================================================================

// Ray-triangle intersection using barycentric coordinates
bool intersect_triangle(Ray const& ray, glm::vec3 const& v0, glm::vec3 const& v1, glm::vec3 const& v2, float* t)
{
    const float eps = 1e-8f;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 normal = glm::cross(edge1, edge2);

    float nd = glm::dot(normal, ray.d);
    if (std::abs(nd) < eps)
        return false;

    // Compute t
    float plane_d = glm::dot(normal, v0);
    float t_tmp   = (plane_d - glm::dot(normal, ray.o)) / nd;

    if (t_tmp < eps)
        return false;

    // Compute intersection
    glm::vec3 p = ray.o + t_tmp * ray.d;

    glm::vec3 vp = p - v0;

    float d00 = glm::dot(edge1, edge1);
    float d01 = glm::dot(edge1, edge2);
    float d11 = glm::dot(edge2, edge2);
    float d20 = glm::dot(vp, edge1);
    float d21 = glm::dot(vp, edge2);

    // barycentric coordinates
    float denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < eps)
        return false;

    float inv_denom = 1.0f / denom;
    float u         = (d11 * d20 - d01 * d21) * inv_denom;
    float v         = (d00 * d21 - d01 * d20) * inv_denom;

    // check if inside
    if (u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f)
    {
        *t = t_tmp;
        // std::cout << "Hit: t=" << t_tmp << " u=" << u << " v=" << v << std::endl;
        return true;
    }

    return false;
}

// for task 6
bool intersect_sphere(Ray const& ray, BoundingSphere const& s)
{
    glm::vec3 oc = ray.o - s.center;

    float a = glm::dot(ray.d, ray.d);
    float b = 2.0f * glm::dot(oc, ray.d);
    float c = glm::dot(oc, oc) - s.radius * s.radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
        return false;

    return true;
}

bool intersect(Scene const& scene, Ray const& ray, Intersection* it, bool test_bounding_spheres)
{
    float closest_t    = std::numeric_limits<float>::max();
    bool  hit = false;

    for (size_t mesh_idx = 0; mesh_idx < scene.meshes.size(); ++mesh_idx)
    {
        Mesh const& mesh = scene.meshes[mesh_idx];

        // bounding sphere optimization
        if (test_bounding_spheres)
        {
            if (!intersect_sphere(ray, scene.bounding_volumes[mesh_idx]))
                continue; // skip mesh
        }

        for (size_t tri_idx = 0; tri_idx < mesh.indices.size(); ++tri_idx)
        {
            glm::u32vec3 const& tri = mesh.indices[tri_idx];

            glm::vec3 const& v0 = mesh.vertices[tri.x];
            glm::vec3 const& v1 = mesh.vertices[tri.y];
            glm::vec3 const& v2 = mesh.vertices[tri.z];

            float t;
            if (intersect_triangle(ray, v0, v1, v2, &t))
            {
                if (t < closest_t)
                {
                    closest_t    = t;
                    hit = true;

                    it->mesh_index      = static_cast<int>(mesh_idx);
                    it->primitive_index = static_cast<int>(tri_idx);
                    it->brdf_index      = mesh.brdf_index;
                    it->emission_index  = mesh.emission_index;
                    it->t               = t;
                    it->p               = ray.o + t * ray.d;

                    // normal
                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;
                    it->normal      = glm::normalize(glm::cross(edge1, edge2));
                }
            }
        }
    }

    return hit;
}


void render_intersection(Scene const& scene, cgtub::PerspectiveCamera const& camera, int width, int height, RenderMode render_mode, 
                        std::vector<glm::vec3>* image)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ray ray = generate_ray(camera, width, height, static_cast<float>(x), static_cast<float>(y));

            Intersection it;
            if (intersect(scene, ray, &it, false))
            {
                // Hitting
                (*image)[y * width + x] = get_intersection_color(render_mode, it);
            }
            else
            {
                // Not Hitting
                (*image)[y * width + x] = glm::vec3(1.0f, 0.0f, 1.0f);
            }
        }
    }
}


glm::vec3 trace_path(Scene const& scene, Ray const& ray, int depth,int max_depth, bool use_brdf_importance_sampling, bool test_bounding_spheres)
{
    if (depth > max_depth)
        return glm::vec3(0.0f);

    ex5::Intersection it;
    if (!intersect(scene, ray, &it, test_bounding_spheres))
        return scene.background;

    glm::vec3 wo = -ray.d;
    glm::vec3 L  = eval_emission(scene, it, wo);
    // std::cout << "depth=" << depth << " emission=" << L.r << "," << L.g << "," << L.b << std::endl;
    
    // bounce if we haven't reached max depth
    if (depth < max_depth && it.brdf_index >= 0)
    {
        glm::vec3 wi;
        float     pdf;

        bool sampled = false;
        if (use_brdf_importance_sampling)
            sampled = sample_direction_from_brdf(scene, it, wo, &wi, &pdf);
        else
            sampled = sample_direction_uniform(it, &wi, &pdf);

        if (sampled && pdf > 0.0f)
        {
            // offset so ray doesn't hit same surface
            const float offset = 1e-5f;
            ex5::Ray    next_ray;
            next_ray.o = it.p + offset * wi;
            next_ray.d = wi;

            glm::vec3 L_in = trace_path(scene, next_ray, depth + 1, max_depth, use_brdf_importance_sampling, test_bounding_spheres);

            glm::vec3 f         = eval_brdf(scene, it, wo, wi);
            float     cos_theta = glm::max(glm::dot(it.normal, wi), 0.0f);

            L += (f * cos_theta / pdf) * L_in;
        }
    }
    return L;
}



void render_pathtracing(Scene const& scene, cgtub::PerspectiveCamera const& camera,int width, int height, int max_depth,int samples_per_pixel, bool use_brdf_importance_sampling, 
                       bool test_bounding_spheres, std::vector<glm::vec3>* image)
{
    samples_per_pixel = std::max(1, samples_per_pixel);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            glm::vec3 sum_radiance(0.0f);

            for (int s = 0; s < samples_per_pixel; ++s)
            {
                float jitter_x = get_uniform_random_value() - 0.5f;
                float jitter_y = get_uniform_random_value() - 0.5f;

                float px = glm::clamp(float(x) + jitter_x, 0.0f, float(width - 1));
                float py = glm::clamp(float(y) + jitter_y, 0.0f, float(height - 1));

                Ray ray = generate_ray(camera, width, height, px, py);
                sum_radiance += trace_path(scene, ray, 1, max_depth, use_brdf_importance_sampling, test_bounding_spheres);

            }

            (*image)[y * width + x] = sum_radiance / float(samples_per_pixel);
        }
    }
}

bool sample_direction_from_brdf(Scene const& scene, Intersection const& it, glm::vec3 const& wo, glm::vec3* wi, float* pdf)
{
    if (!wi || !pdf)
        return false;
    if (it.brdf_index < 0)
        return false;

    ex5::BRDF const& brdf = scene.brdfs[it.brdf_index];
    glm::vec3        n    = glm::normalize(it.normal);

    if (brdf.type == BRDFType::Mirror)
    {
        // reflection
        glm::vec3 incoming = -glm::normalize(wo);
        *wi                = glm::normalize(incoming - 2.0f * glm::dot(incoming, n) * n);
        *pdf               = 1.0f;
        return true;
    }

    if (brdf.type == BRDFType::Diffuse)
    {
        float u1 = get_uniform_random_value();
        float u2 = get_uniform_random_value();

        float r   = std::sqrt(u1);
        float phi = 2.0f * glm::pi<float>() * u2;

        // local coordinates
        float x_local = r * std::cos(phi);
        float y_local = r * std::sin(phi);
        float z_local = std::sqrt(std::max(0.0f, 1.0f - u1));

        // build tangent frame
        glm::vec3 tangent, bitangent;
        construct_frame(n, &tangent, &bitangent);

        // convert to world space
        *wi = glm::normalize(x_local * tangent + y_local * bitangent + z_local * n);

        float cos_theta = glm::max(0.0f, glm::dot(n, *wi));
        *pdf            = cos_theta / glm::pi<float>();

        // std::cout << "pdf=" << *pdf << " cos=" << cos_theta << std::endl;

        return cos_theta > 0.0f;
    }

    return false;
}


} // namespace ex5