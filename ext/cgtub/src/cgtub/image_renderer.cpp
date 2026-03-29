#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cgtub/canvas.hpp>
#include <cgtub/gl_wrap.hpp>
#include <cgtub/image_renderer.hpp>
#include <cgtub/log.hpp>

namespace cgtub
{

constexpr const char* vshader = R"glsl(
    #version 330

    out vec2 uv;

    void main()
    {
        // For vertex indices 0, 1, 2, 3 
        // generates uvs (0, 0), (1, 0), (0, 1), (1, 1)
        uv            = vec2(gl_VertexID & 1, (gl_VertexID & 2) >> 1);
        vec3 position = vec3(2.f * uv.x - 1.f, 2.f * uv.y - 1.f, 0.f);

        gl_Position = vec4(position, 1);
    }
)glsl";

constexpr const char* fshader = R"glsl(
    #version 330

    uniform sampler2D image;

    in vec2 uv;

    out vec4 color;

    void main()
    {
        color = texture(image, uv); //vec4(0.f, 1.f, 0.f, 1.f);
    }
)glsl";

ImageRenderer::ImageRenderer(Canvas& canvas)
    : m_canvas(canvas)
{
    glGenTextures(1, &m_texture);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the vertex array object
    // NOTE: This is a dummy and remains empty because the full screen
    //       quad is directly generated in the vertex shader.
    glGenVertexArrays(1, &m_vao);

    create_program(vshader, fshader, &m_program);
}

void ImageRenderer::update_texture(std::span<glm::vec3 const> image, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Update the texture buffer
    GLint actualWidth(0u);
    GLint actualHeight(0u);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &actualWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &actualHeight);
    if (actualWidth != width || actualHeight != height)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, image.data());
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, image.data());
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageRenderer::render(std::span<glm::vec3 const> image, int width, int height)
{
    if (image.empty())
    {
        log_message(LogLevel::Warn, "ImageRenderer::render(): No pixel colors provided for the image. Did you forget to populate an array?");
        return;
    }

    if (width < 0 || height < 0)
    {
        log_message(LogLevel::Error, "ImageRenderer::render(): Image has negative extent with (width, height) = (%d, %d), nothing is rendered.", width, height);
        return;
    }

    if (width == 0 || height == 0)
    {
        log_message(LogLevel::Warn, "ImageRenderer::render(): Image has zero extent with (width, height) = (%d, %d), nothing is rendered.", width, height);
        return;
    }

    Rect viewport = m_canvas.viewport();
    if (viewport.width == 0 || viewport.height == 0)
    {
        log_message(LogLevel::Trace, "ImageRenderer::render(): canvas has size 0, nothing is rendererd");
        return;
    }

    update_texture(image, width, height);

    set_viewport(m_canvas.window(), m_canvas.viewport());

    glUseProgram(m_program);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    gl_check_error(__FILE__, __LINE__);

    glBindVertexArray(0);
    glUseProgram(0);
}

} // namespace cgtub