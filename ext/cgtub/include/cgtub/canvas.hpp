#pragma once

#include "glm/glm.hpp"

#include "cgtub/fwd.hpp"
#include "cgtub/primitives.hpp"

// Forward declarations
struct GLFWwindow;

namespace cgtub
{

/**
 * \brief A (logical) subregion of a window.
 *
 * Some settings require rendering only to a subregion of a window, e.g., 
 * a split screen application that shows different parts of the scene
 * in the left and right half of the window:
 * 
 *          window
 *  _____________________
 * |          |          |
 * |          |          |
 * | canvas_1 | canvas_2 |
 * |          |          |
 * |__________|__________|
 * 
 * Such a rendering subregion can be implemented using a canvas.
 * 
 * Its region is defined using the (x, y) coordinates of canvas origin
 * (its lower left corner) and a width and height. 
 * 
 *          window
 *  _________________________
 * |                         |
 * |      width >_           |
 * |     |        |          |
 * |     | canvas | ^ height |
 * |     .________|          |
 * |  (x, y)                 |
 * |_________________________|
 * 
 * The (x, y) coordinates, the width, and the height are normalized,
 * meaning they are relativ to the current size (in pixels) of the window.
 * 
 * Examples: 
 * - A canvas with x=0, y=0, width=1, height=1 covers the full window
 * - A canvas with x=0.5, y=0, width=0.5, height=1 covers the right half of the window 
 *   (like \c canvas_2 above)
 * - A canvas with x=0, y=0.5, width=1, height=1 covers the top half of the window but
 *   extends beyond the top window border, with half of it being invisible.
 */
class Canvas
{
public:
    /**
     * \brief Construct a new canvas for a \c GLFWwindow.
     * 
     * \param[in] window The \c GLFWwindow the canvas is attached to.
     * \param[in] extent The (normalized) subregion of the canvas, i.e., x, y, width, and height
     *                   (for details see the documentation of \ref Canvas).
     */
    Canvas(GLFWwindow* window, Extent const& extent = Extent{.x = 0.f, .y = 0.f, .width = 1.f, .height = 1.f});

    // Handle events (resizing window, etc.)
    void update(float dt, EventDispatcher const* dispatcher);

    // The window the canvas is attached to.
    GLFWwindow* window();

    // The region defined by the canvas, in pixels.
    Rect viewport(bool return_size_on_window = false) const;

    /**
     * \brief Clear the canvas (region of the window) with a color. 
     * 
     * \param[in] color The clear color. The default is black (0, 0, 0).
     */
    void clear(glm::vec3 const& color = glm::vec3(0));

    /**
     * \brief Convert the position of a pixel (x, y) that is relative to the 
     *        window origin to a position relative to the canvas origin.
     * 
     * \param[in,out] x position, will be set to the output x position
     * \param[in,out] y position, will be set to the output y position
     */
    void map_to_canvas(int* x, int* y) const;

    // Indicator if a window pixel position (x, y) is inside the canvas region
    bool is_inside(int x, int y) const;

    
    void handle_resize();

private:
    GLFWwindow* m_window;
    Extent      m_extent;
    Rect        m_viewport;
    float       m_pixel_scaling[2];
};

} // namespace cgtub