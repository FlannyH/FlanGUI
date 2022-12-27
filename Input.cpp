#include "Input.h"
#include <fstream>
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
            _mouse_pos_prev = _mouse_pos_curr;
            _mouse_pos_curr = { x, y };
        }

        // Get window position
        {
            int x, y;
            glfwGetWindowPos(window, &x, &y);
            _window_pos = { x, y };
        }

        // Get mouse buttons
        {
            for (int i = 0; i < 3; i++)
            {
                _mouse_button_prev[i] = _mouse_button_curr[i];
                _mouse_button_curr[i] = glfwGetMouseButton(window, i);
            }
        }

        // Reset scroll
        _mouse_wheel_buffer = _mouse_wheel;
        _mouse_wheel = 0;

        // Set mouse cursor visibility
        glfwSetInputMode(window, GLFW_CURSOR, _mouse_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
    
    void Input::scroll_callback(GLFWwindow* window, [[maybe_unused]] double x, double y) {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->_mouse_wheel += static_cast<float>(y);
    }

    glm::vec2 Input::mouse_pos(const MouseRelative rel) const {
        switch (rel) {
        case MouseRelative::absolute:
            return _mouse_pos_curr + _window_pos;
        case MouseRelative::window:
            return _mouse_pos_curr;
        case MouseRelative::relative:
            return _mouse_pos_curr - _mouse_pos_prev;
        }
        return {-1, -1};
    }

    bool Input::mouse_held(const int button) const {
        return static_cast<bool>(_mouse_button_curr[button]);
    }

    bool Input::mouse_down(const int button) const {
        return _mouse_button_curr[button] && !_mouse_button_prev[button];
    }

    bool Input::mouse_up(const int button) const {
        return !_mouse_button_curr[button] && _mouse_button_prev[button];
    }

    float Input::mouse_wheel() const {
        return _mouse_wheel_buffer;
    }

    void Input::mouse_visible(const bool value) {
        _mouse_visible = value;
    }
}
