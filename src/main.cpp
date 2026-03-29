#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <span>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "cgtub/canvas.hpp"
#include "cgtub/event_dispatcher.hpp"
#include "cgtub/gl_wrap.hpp"
#include "cgtub/image_renderer.hpp"
#include "cgtub/primitives.hpp"
#include "cgtub/simple_renderer.hpp"

#include "helper.hpp"

int main(int argc, char** argv)
{
    // Create a GLFW window and an OpenGL context
    // The event dispatcher records incoming events for a window
    // (e.g. change of size or mouse cursor movement)
    GLFWwindow*             window     = nullptr;
    cgtub::EventDispatcher* dispatcher = nullptr;
    if (!cgtub::init(800, 480, "CG1", &window, &dispatcher))
    {
        std::cerr << "Failed to initialize OpenGL window" << std::endl;
        return EXIT_FAILURE;
    }

    // Two canvases: one for the left side of the window (the rasterized scene) and one for the right side (the pathtraced scene)
    float         margin = 0.01f;
    cgtub::Canvas canvas_left(window, cgtub::Extent{.x = margin, .y = margin, .width = 0.5f - margin, .height = 1.f - 2 * margin});
    cgtub::Canvas canvas_right(window, cgtub::Extent{.x = 0.5f + margin, .y = margin, .width = 0.5f - 2 * margin, .height = 1.f - 2 * margin});

    cgtub::SimpleRenderer renderer_left(canvas_left);
    cgtub::ImageRenderer  renderer_right(canvas_right);

    // In each frame, we want to generate image data
    // (i.e., a color for each pixel), which is then passed to the ImageRenderer.
    // The generated image should match the canvas aspect ratio,
    // so it's size (width, height) should be a multiple of the canvas'.
    cgtub::Rect viewport = canvas_right.viewport(true);
    int         width    = viewport.width;
    int         height   = viewport.height;

    // For large screen resolutions, the canvas resolution can be very
    // high and low-performance hardware may struggle to generate
    // the image data interactively. We therefore introduce the
    // `subsampling_rate` that reduces the number of pixels to generate.
    // If the `subsampling_rate` is 1, then `image` has as many pixels
    // as the canvas. If it is n, every pixel in `image` corresponds to
    // n^2 pixels on the canvas.
    int subsampling_rate = 8;
    width /= subsampling_rate;
    height /= subsampling_rate;

    // The image data itself (i.e. color for each pixel) is simply an array of colors.
    // For a pixel (x,y) the color is accessed as image[y*width + x].
    std::vector<glm::vec3> image(width * height);

    // ... this is an auxiliary buffer that can be used for accumulated rendering.
    // It is automatically cleared when the camera moves
    std::vector<glm::vec3> image_accumulated(width * height);
    int                    num_accumulated_samples(0);

    // Create the Cornell Box scene
    ex5::Scene scene;
    ex5::setup_scene(&scene);

    // Application state
    ex5::RenderMode render_mode                  = ex5::RenderMode::Distance;
    bool            automatic_rendering          = false;
    bool            accumulate_samples           = false;
    int             samples_per_pixel            = 4;
    int             max_depth                    = 3;
    bool            use_brdf_importance_sampling = false;
    bool            test_bounding_spheres        = false;
    bool            show_bounding_spheres        = false;

    glm::mat4 view_matrix_previous = renderer_left.camera().view();

    float time = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
        cgtub::begin_frame(window);

        // Track current time and elapsed time between frames (dt)
        float now = static_cast<float>(glfwGetTime());
        float dt  = now - time;
        time      = now;

        // Poll and record window events (resizing, key inputs, etc.)
        // The dispatcher is implicitly connected to the window and receives these events.
        dispatcher->poll_window_events();

        // The canvas and renderer must react to incoming events (resizing, user inputs, ...)
        canvas_left.update(dt, dispatcher);
        canvas_right.update(dt, dispatcher);
        renderer_left.update(dt, dispatcher);

        bool            do_render                = false;
        ex5::GuiChanges gui_changes              = ex5::gui(&subsampling_rate, &render_mode, &do_render, &automatic_rendering, &max_depth, &samples_per_pixel, &accumulate_samples, &use_brdf_importance_sampling, &test_bounding_spheres, &show_bounding_spheres);

        if (ex5::has_gui_changed_parameter(gui_changes, 0) || dispatcher->was_framebuffer_resized())
        {
            // If the window has been resized or subsampling was changed,
            // the image size needs to be adapted (unless it's zero)
            cgtub::Rect viewport = canvas_right.viewport(true);
            if (viewport.width != 0 && viewport.height != 0)
            {
                width  = viewport.width / subsampling_rate;
                height = viewport.height / subsampling_rate;

                image.resize(width * height);
                std::fill(image.begin(), image.end(), glm::vec3(0));

                image_accumulated.resize(width * height);
                num_accumulated_samples = 0;
                std::fill(image_accumulated.begin(), image_accumulated.end(), glm::vec3(0));
            }
        }

        // If `max_depth` or `use_brdf_importance_sampling` are changed or if the camera moves between frames, reset the currently accumulated image.
        uint32_t mutating_params[]       = {4, 7};
        bool     reset_accumulated_image = std::any_of(std::begin(mutating_params), std::end(mutating_params), [&](uint32_t idx)
                                                       { return ex5::has_gui_changed_parameter(gui_changes, idx); }) ||
                                       view_matrix_previous != renderer_left.camera().view();
        if (reset_accumulated_image)
        {
            num_accumulated_samples = 0;
            std::fill(image_accumulated.begin(), image_accumulated.end(), glm::vec3(0));
            view_matrix_previous = renderer_left.camera().view();
        }

        cgtub::clear(window, 0.05f, 0.05f, 0.05f, 1);

        // The left view rasterizes the scene and shows a rough overview
        canvas_left.clear(scene.background);
        ex5::rasterize_scene(renderer_left, scene, show_bounding_spheres);

        // The right view displays the generic `image` (e.g. filled with radiance from path tracing
        canvas_right.clear();
        renderer_right.render(image, width, height);
        if (do_render || automatic_rendering)
        {
            if (render_mode != ex5::RenderMode::PathTracing)
            {
                ex5::render_intersection(scene, renderer_left.camera(),
                                         width, height, render_mode, &image);
            }
            else
            {
                ex5::render_pathtracing(scene, renderer_left.camera(),
                                        width, height, max_depth, samples_per_pixel, use_brdf_importance_sampling, test_bounding_spheres, &image);
            }
        }
        cgtub::end_frame(window);
    }

    cgtub::uninit(window, dispatcher);

    return EXIT_SUCCESS;
}
