#include "Input.h"

namespace Flan {
    Input::Input(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, scroll_callback);
    }

    void Input::update(GLFWwindow* window) {
        // Get mouse position
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            m_mouse_pos_prev = m_mouse_pos_curr;
            m_mouse_pos_curr = { x, y };
        }

        // Get window position
        {
            int x, y;
            glfwGetWindowPos(window, &x, &y);
            m_window_pos = { x, y };
        }

        // Get mouse buttons
        {
            for (int i = 0; i < 3; i++)
            {
                m_mouse_button_prev[i] = m_mouse_button_curr[i];
                m_mouse_button_curr[i] = glfwGetMouseButton(window, i);
            }
        }

        // Reset scroll
        m_mouse_wheel_buffer = m_mouse_wheel;
        m_mouse_wheel = 0;

        // Set mouse cursor visibility
        glfwSetInputMode(window, GLFW_CURSOR, m_mouse_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
    
    void Input::scroll_callback(GLFWwindow* window, [[maybe_unused]] double x, double y) {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->m_mouse_wheel += static_cast<float>(y);
    }

    glm::vec2 Input::mouse_pos(const MouseRelative rel) const {
        switch (rel) {
        case MouseRelative::absolute:
            return m_mouse_pos_curr + m_window_pos;
        case MouseRelative::window:
            return m_mouse_pos_curr;
        case MouseRelative::relative:
            return m_mouse_pos_curr - m_mouse_pos_prev;
        }
        return {-1, -1};
    }

    bool Input::mouse_held(const int button) const {
        return static_cast<bool>(m_mouse_button_curr[button]);
    }

    bool Input::mouse_down(const int button) const {
        return m_mouse_button_curr[button] && !m_mouse_button_prev[button];
    }

    bool Input::mouse_up(const int button) const {
        return !m_mouse_button_curr[button] && m_mouse_button_prev[button];
    }

    float Input::mouse_wheel() const {
        return m_mouse_wheel_buffer;
    }

    void Input::mouse_visible(const bool value) {
        m_mouse_visible = value;
    }
}
