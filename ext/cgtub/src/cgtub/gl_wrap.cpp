#include <iostream>
#include <vector>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "cgtub/event_dispatcher.hpp"
#include "cgtub/gl_wrap.hpp"
#include "cgtub/log.hpp"

namespace cgtub
{

void error_callback(int error, const char* message)
{
    log_message(LogLevel::Error, "[GLFW Error] %s", message);
}

void gl_check_error(const char* file, unsigned int line)
{
    GLenum errorCode = glGetError();

    while (errorCode != GL_NO_ERROR)
    {
        std::string fileString(file);
        std::string error = "unknown error";

        // clang-format off
        switch (errorCode) {
            case GL_INVALID_ENUM:      error = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:     error = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: error = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:    error = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:   error = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:     error = "GL_OUT_OF_MEMORY"; break;
        }
        // clang-format on

        log_message(LogLevel::Error, "OpenGL Error (%s:L.%d): %s", file, line, error.c_str());
        errorCode = glGetError();
    }
}

bool init(unsigned int width, unsigned int height, std::string_view title, GLFWwindow** window, EventDispatcher** dispatcher)
{
    if (!window)
    {
        log_message(LogLevel::Error, "init(): 'window' is null, cannot initialize.");
        return false;
    }

    if (!dispatcher)
    {
        log_message(LogLevel::Error, "init(): 'dispatcher' is null, cannot initialize.");
        return false;
    }

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        log_message(LogLevel::Error, "Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_SRGB_CAPABLE, 1);

    // OpenGL 4.1 core profile is required for compatibility with MacOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    *window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (*window == nullptr)
    {
        log_message(LogLevel::Error, "Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_FRAMEBUFFER_SRGB);

    // Register an event dispatcher
    *dispatcher = new EventDispatcher(*window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(*window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    return true;
}

void uninit(GLFWwindow* window, EventDispatcher* dispatcher)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete dispatcher;

    glfwDestroyWindow(window);
    glfwTerminate();
}

void begin_frame(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void end_frame(GLFWwindow* window)
{
    // Actually render the GUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Display the rendered content on the monitor
    glfwSwapBuffers(window);
}

void clear(GLFWwindow* window, float r, float g, float b, float a, std::optional<Rect> viewport)
{
    glfwMakeContextCurrent(window);

    GLboolean hasScissor = glIsEnabled(GL_SCISSOR_TEST);
    if (viewport)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(static_cast<GLint>(viewport->x), static_cast<GLint>(viewport->y),
                  static_cast<GLsizei>(viewport->width), static_cast<GLsizei>(viewport->height));
    }

    glClearColor(r, g, b, a);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Just clear depth with color..

    if (viewport)
    {
        // Restore scissor state
        if (hasScissor)
        {
            glEnable(GL_SCISSOR_TEST);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
    }
}

void set_viewport(GLFWwindow* window, std::optional<Rect> viewport)
{
    glfwMakeContextCurrent(window);

    if (viewport)
        glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }
}

bool compile_shader(GLenum type, const char* source, GLuint* shader)
{
    *shader = glCreateShader(type);
    if (*shader == 0)
    {
        log_message(LogLevel::Error, "Unable to create shader");
        return false;
    }

    // Set shader source and compile
    glShaderSource(*shader, 1, (const GLchar**)&source, nullptr);
    glCompileShader(*shader);

    // Check compilation
    GLint status;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logsize = 0;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logsize);

        std::vector<GLchar> log(logsize + 1);
        glGetShaderInfoLog(*shader, logsize, &logsize, log.data());

        log_message(LogLevel::Error, "Unable to compile shader:\n %s", log.data());

        glDeleteShader(*shader);
        *shader = 0;

        return false;
    }

    return true;
}

bool link_program(GLuint vshader, GLuint fshader, GLuint* program)
{
    *program = glCreateProgram();

    glAttachShader(*program, vshader);
    glAttachShader(*program, fshader);
    glLinkProgram(*program);

    GLint status;
    glGetProgramiv(*program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logsize = 0;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &logsize);

        std::vector<GLchar> log(logsize + 1);
        glGetProgramInfoLog(*program, logsize, nullptr, log.data());

        log_message(LogLevel::Error, "Unable to link program:\n %s", log.data());

        glDeleteProgram(*program);
        *program = 0;

        return false;
    }

    return true;
}

bool create_program(const char* vsource, const char* fsource, GLuint* program)
{
    if (!vsource || std::strlen(vsource) == 0)
    {
        log_message(LogLevel::Error, "create_program(): no vertex shader source provided");
        return false;
    }

    if (!fsource || std::strlen(fsource) == 0)
    {
        log_message(LogLevel::Error, "create_program(): no fragment shader source provided");
        return false;
    }

    if (!program)
    {
        log_message(LogLevel::Error, "create_program(): output pointer to program id is null");
        return false;
    }

    bool   success = true;
    GLuint vshader = 0;
    GLuint fshader = 0;
    success &= compile_shader(GL_VERTEX_SHADER, vsource, &vshader);
    success &= compile_shader(GL_FRAGMENT_SHADER, fsource, &fshader);

    GLuint program_ = 0;
    if (success)
    {
        success &= link_program(vshader, fshader, &program_);
    }

    if (!success)
    {
        // Clean up...
        for (GLuint shader : {vshader, fshader})
        {
            if (shader != 0)
                glDeleteShader(shader);
        }

        if (program_ != 0)
            glDeleteProgram(program_);
    }
    else
    {
        *program = program_;
    }

    return success;
}

#define COMMA ,

#define IMPLEMENT_UNIFORM_SETTER(type, gltype, ptr)                          \
    template<>                                                               \
    void set_uniform<type>(GLuint program, GLint location, type const& value) \
    {                                                                        \
        glUseProgram(program);                                               \
        glUniform##gltype(location, 1, ptr(value));                          \
        glUseProgram(0u);                                                    \
    }

IMPLEMENT_UNIFORM_SETTER(float, 1fv, &)
IMPLEMENT_UNIFORM_SETTER(double, 1dv, &)
IMPLEMENT_UNIFORM_SETTER(int, 1iv, &)
IMPLEMENT_UNIFORM_SETTER(unsigned int, 1uiv, &)

// FIXME: This does not compile. Why?
// IMPLEMENT_UNIFORM_SETTER(glm::vec1, 1fv, glm::value_ptr)
// IMPLEMENT_UNIFORM_SETTER(glm::dvec1, 1dv, glm::value_ptr)
// IMPLEMENT_UNIFORM_SETTER(glm::i32vec1, 1iv, glm::value_ptr)
// IMPLEMENT_UNIFORM_SETTER(glm::u32vec1, 1uiv, glm::value_ptr)

IMPLEMENT_UNIFORM_SETTER(glm::vec2, 2fv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::dvec2, 2dv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::i32vec2, 2iv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::u32vec2, 2uiv, glm::value_ptr)

IMPLEMENT_UNIFORM_SETTER(glm::vec3, 3fv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::dvec3, 3dv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::i32vec3, 3iv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::u32vec3, 3uiv, glm::value_ptr)

IMPLEMENT_UNIFORM_SETTER(glm::vec4, 4fv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::dvec4, 4dv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::i32vec4, 4iv, glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::u32vec4, 4uiv, glm::value_ptr)

IMPLEMENT_UNIFORM_SETTER(glm::mat2, Matrix2fv, GL_FALSE COMMA glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::mat3, Matrix3fv, GL_FALSE COMMA glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::mat4, Matrix4fv, GL_FALSE COMMA glm::value_ptr)

IMPLEMENT_UNIFORM_SETTER(glm::dmat2, Matrix2dv, GL_FALSE COMMA glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::dmat3, Matrix3dv, GL_FALSE COMMA glm::value_ptr)
IMPLEMENT_UNIFORM_SETTER(glm::dmat4, Matrix4dv, GL_FALSE COMMA glm::value_ptr)

void bind_texture(GLuint program, GLint location, GLuint texture)
{
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(0u);
}

} // namespace cgtub