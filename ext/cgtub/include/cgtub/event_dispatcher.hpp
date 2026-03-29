#pragma once

#include <functional>
#include <unordered_map>

// Forward declarations
struct GLFWwindow;

namespace cgtub
{

/**
 * \brief A record of input events associated with a \c GLFWwindow. 
 * 
 * Typically, an instance holds all events that occured in a *single* frame.
 */
struct InputEvents
{
    inline InputEvents(GLFWwindow* window)
        : window(window)
    {
    }

    /**
     * \brief Get the recorded action for a mouse button.
     * 
     * \param[in] button Identifier of the mouse button (e.g. GLFW_MOUSE_BUTTON_1).
     * 
     * \return One of {GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT} if there is a recorded action, else -1.
     */
    inline int button(int button) const
    {
        auto it = buttons.find(button);
        return it != std::end(buttons) ? it->second : -1;
    }

    /**
     * \brief Get the recorded action for a key on the keyboard.
     * 
     * \param key[in] Identifier of the key (e.g. GLFW_KEY_ENTER).
     *  
     * \return One of {GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT} if there is a recorded action, else -1.
     */
    inline int key(int key) const
    {
        auto it = keys.find(key);
        return it != std::end(keys) ? it->second : -1;
    }

    inline void clear()
    {
        keys.clear();
        buttons.clear();
        scroll.xoffset = 0.f;
        scroll.yoffset = 0.f;
    }

    struct
    {
        float xoffset{0.f}; // Amount of x-scroll of the mouse wheel
        float yoffset{0.f}; // Amount of y-scroll of the mouse wheel
    } scroll;

    GLFWwindow*                  window;
    std::unordered_map<int, int> keys;
    std::unordered_map<int, int> buttons;
};

/** 
 * \brief Abstraction for receiving and distributing window events.
 * 
 * An \c EventDispatcher is attached to a single \c GLFWwindow.
 * It is meant to receive and record the window events that occur in a single frame.
 * 
 * Usage example:
 * \code{.cpp}
 * while(...)
 * {
 *     dispatcher.poll_window_events();
 * 
 *     // Get a handle to the input events of this frame
 *     InputEvents const& inputs = dispatcher.inputs();
 * 
 *     ... // React to recorded events using `inputs`
 * }
 * \endcode
 * 
 */
class EventDispatcher
{
public:
    EventDispatcher(GLFWwindow* window);

    /**
     * \brief Poll and record events of the associated window.
     * 
     * This clears all previously recorded events by calling \ref EventDispatcher::clear().
     */
    void poll_window_events();

    // Clear all previously recorded events.
    void clear();

    // Indicator if the window's framebuffer was resized.
    bool was_framebuffer_resized() const;

    GLFWwindow* window();

    InputEvents const& inputs() const;

private:
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    bool        m_was_framebuffer_resized;
    InputEvents m_inputs;
};

} // namespace cgtub