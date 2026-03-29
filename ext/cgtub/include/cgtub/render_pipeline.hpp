#pragma once

#include <string>
#include <typeinfo>

#include <cgtub/attribute_buffer.hpp>
#include <cgtub/gl_wrap.hpp>
#include <cgtub/log.hpp>
#include <cgtub/texture_buffer.hpp>

namespace cgtub
{

#define RENDER_PIPELINE_CHECK_NO_PROGRAM(method_name)                                       \
    if (m_program == 0u)                                                                    \
    {                                                                                       \
        log_message(LogLevel::Debug, "GLRenderPipeline::" method_name ": no active shaders"); \
        return false;                                                                       \
    }

/**
 * Abstraction of the OpenGL render pipeline implemented by modern GPUs.
 */
class GLRenderPipeline
{
public:
    GLRenderPipeline(GLFWwindow* window);

    virtual ~GLRenderPipeline();

    // Delete move and copy constructors/operators
    GLRenderPipeline(GLRenderPipeline const&) = delete;
    GLRenderPipeline(GLRenderPipeline&&) = delete;
    GLRenderPipeline& operator=(GLRenderPipeline const&) = delete;
    GLRenderPipeline& operator=(GLRenderPipeline&&)      = delete;

    /**
     * \brief Set the vertex and fragment shaders used by the pipeline.
     * 
     * Internally, the GLSL shader code is compiled and linked, which may fail,
     * for example, if the shader code contains syntax errors.
     * 
     * \param[in] vertex_shader_code   GLSL code of the vertex shader
     * \param[in] fragment_shader_code GLSL code of the fragment shader
     * 
     * \return True if the shaders were successfully compiled and set, false otherwise.
     */
    bool set_shaders(std::string const& vertex_shader_code, std::string const& fragment_shader_code);

    // Query if the currently active shaders use an attribute with the given `name`
    bool has_attribute(std::string const& name);

    /**
     * \brief Bind data for a vertex attribute with a given name to the pipeline.
     * 
     * Example:
     * 
     * Given a GLSL vertex shader with the input attribute `foo`
     * ```
     * in vec3 foo;
     * ```
     * 
     * The C++ code to bind data to this attribute is:
     * ```
     * GLAttributeBuffer<glm::vec3> foo_buffer;
     * foo_buffer.upload(...);
     * 
     * ...
     * 
     * pipeline.bind_attribute("foo", foo_buffer);
     * ```
     * 
     * NOTE: The call to `pipeline.bind_attribute` does *not* copy data, so uploading new
     * data to the attribute buffer after the call *will* alter the data accessed by the pipeline.
     * 
     * \param[in] name   Name of the attribute as used in the vertex shader
     * \param[in] buffer Buffer containing the data that is bound to the attribute
     * 
     * \return True if the buffer data was successfully bound, false otherwise.
     */
    template<typename T>
    bool bind_attribute(std::string const& name, GLAttributeBuffer<T> const& buffer)
    {
        RENDER_PIPELINE_CHECK_NO_PROGRAM("bind_attribute")

        GLint location = glGetAttribLocation(m_program, name.data());

        if (location < 0)
        {
            log_message(LogLevel::Error, "GLRenderPipeline::bind_attribute<%s> Attribute '%s' is not found in the shaders. Unused attributes are removed as an optimization (is it maybe unused?)",
                        typeid(T).name(), name.c_str());
            return false;
        }

        ::cgtub::bind_attribute<T>(m_vao, location, buffer.gl());

        return true;    
    }

    // Query if the currently active shaders use a uniform with the given `name`
    bool has_uniform(std::string const& name);

    /**
     * \brief Set data for a uniform with a given name to the pipeline.
     *
     * Example:
     *
     * Given a GLSL vertex or fragment shader with the uniform `bar`
     * ```
     * uniform mat4 bar;
     * ```
     *
     * The C++ code to set data for this uniform is:
     * ```
     * glm::mat4 bar_matrix = ...;
     * 
     * ...
     * 
     * pipeline.set_uniform("bar", bar_matrix);
     * ```
     * 
     * NOTE: The call to `pipeline.set_uniform` copies the data, so overwriting `bar_matrix`
     * in the example above after the call will not affect the uniform value of the pipeline.
     *
     * \param[in] name  Name of the uniform as used in the shaders
     * \param[in] value Value to set the uniform to
     *
     * \return True if the value was successfully set, false otherwise.
     */
    template<typename T>
    bool set_uniform(std::string const& name, T const& value)
    {
        RENDER_PIPELINE_CHECK_NO_PROGRAM("set_uniform")

        GLint location = glGetUniformLocation(m_program, name.c_str());

        if (location < 0)
        {
            log_message(LogLevel::Error, "GLRenderPipeline::set_uniform<%s> Uniform '%s' is not found in the shaders. Unused uniforms are removed as an optimization (is it maybe unused?)",
                        typeid(T).name(), name.c_str());
            return false;
        }

        ::cgtub::set_uniform(m_program, location, value);

        return true;
    }

    // Query if the currently active shaders use a texture with the given `name`
    bool has_texture(std::string const& name);

    /**
     * \brief Bind texture data to a texture sampler with the given name to the pipeline.
     *
     * Example:
     *
     * Given a GLSL shader with the uniform sampler `foobar`
     * ```
     * uniform sampler2D foobar;
     * ```
     *
     * The C++ code to bind data for this texture sampler is:
     * ```
     * GLTextureBuffer foobar_buffer;
     * foobar_buffer.upload(...);
     *
     * ...
     *
     * pipeline.bind_texture("foobar", foobar_buffer);
     * ```
     *
     * NOTE: The call to `pipeline.bind_texture` does *not* copy data, so uploading new
     * data to the texture buffer after the call *will* alter the data accessed by the pipeline.
     *
     * \param[in] name   Name of the texture sampler as used in the shaders
     * \param[in] buffer Buffer containing the data that is bound to the texture sampler
     *
     * \return True if the buffer data was successfully bound, false otherwise.
     */
    bool bind_texture(std::string const& name, GLTextureBuffer const& texture);

    /**
     * \brief Render the given triangles, represented by the index buffer, with the currently bound shaders, uniforms, attributes, and textures.
     * 
     * \param[in] indices  Index buffer that determines the triangles/primitives
     * \param[in] viewport Sub-region of the window to render to (by default render to the full window).
     */
    void render_indexed(GLAttributeBuffer<glm::u32vec3> const& indices, std::optional<Rect> viewport = std::nullopt);

private:
    GLFWwindow* m_window;
    GLuint      m_program;
    GLuint      m_vao;
};

} // namespace cgtub