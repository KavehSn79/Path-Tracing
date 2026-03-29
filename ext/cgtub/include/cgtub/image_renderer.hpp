#pragma once

#include <span>

#include <glm/glm.hpp>

#include <glad/glad.h>

#include <cgtub/fwd.hpp>
#include <cgtub/primitives.hpp>

namespace cgtub
{

/**
 * \brief A renderer for displaying images on a canvas.
 * 
 *  An \c ImageRenderer is always attached to a canvas, which is used as a render target.
 */
class ImageRenderer
{
public:
    // Construct a renderer that renders to the given canvas.
    ImageRenderer(Canvas& canvas);

    /**
     * \brief Render an image to the canvas (filling the full canvas).
     *
     * The \c image array contains a color for each pixel of the 
     * image in a linear layout: the RGB color of a pixel (x,y) is accessed 
     * as image[y * width + x].
     * 
     * \param[in] image  Color for each pixel in the image in a linear layout.
     * \param[in] width  The width of the image.
     * \param[in] height The height of the image.
     */
    void render(std::span<glm::vec3 const> image, int width, int height);

private:
    void update_texture(std::span<glm::vec3 const> image, int width, int height);

    Canvas& m_canvas;
    GLuint  m_texture{0u};
    GLuint  m_program{0u};
    GLuint  m_vao{0u};
};

} // namespace cgtub