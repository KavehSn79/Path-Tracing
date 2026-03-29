#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "cgtub/event_dispatcher.hpp"

namespace cgtub
{

EventDispatcher::EventDispatcher(GLFWwindow* window)
    : m_window(window)
    , m_was_framebuffer_resized(false)
    , m_inputs(window)
{
    void* userPtr = glfwGetWindowUserPointer(m_window);
    if (userPtr)
    {
        // TODO: Warn, another event dispatcher set?
    }
    glfwSetWindowUserPointer(m_window, this);

    glfwSetMouseButtonCallback(m_window, EventDispatcher::mouse_button_callback);
    glfwSetScrollCallback(m_window, EventDispatcher::scroll_callback);
    glfwSetKeyCallback(m_window, EventDispatcher::key_callback);
    glfwSetFramebufferSizeCallback(m_window, EventDispatcher::framebuffer_size_callback);
}

void EventDispatcher::poll_window_events()
{
    clear();
    glfwPollEvents();
}

void EventDispatcher::clear()
{
    m_inputs.clear();
    m_was_framebuffer_resized = false;
}

bool EventDispatcher::was_framebuffer_resized() const
{
    return m_was_framebuffer_resized;
}

GLFWwindow* EventDispatcher::window()
{
    return m_window;
}
InputEvents const& EventDispatcher::inputs() const
{
    return m_inputs;
}

void EventDispatcher::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    EventDispatcher& self         = *reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
    self.m_inputs.buttons[button] = action;
}

void EventDispatcher::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    EventDispatcher& self        = *reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
    self.m_inputs.scroll.xoffset = static_cast<float>(xoffset);
    self.m_inputs.scroll.yoffset = static_cast<float>(yoffset);
}

void EventDispatcher::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    EventDispatcher& self   = *reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
    self.m_inputs.keys[key] = action;
}

void EventDispatcher::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    EventDispatcher& self          = *reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
    self.m_was_framebuffer_resized = true;
}

} // namespace cgtub