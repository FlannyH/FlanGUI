#pragma once
#include "glfw/glfw3.h"
#include "glm/vec2.hpp"

namespace Flan {
    enum class MouseRelative {
        absolute = 0,
        window,
        relative,
    };

    class Input {
    public:
        explicit Input(GLFWwindow* window);
        void update(GLFWwindow* window); // Get input data. Should be called at the end of the main loop to get proper mouse wheel support.
        static void scroll_callback(GLFWwindow* window, [[maybe_unused]] double x, double y);
        void mouse_visible(bool value);
        [[nodiscard]] glm::vec2 mouse_pos(MouseRelative rel) const; // Get the current mouse position
        [[nodiscard]] bool mouse_held(int button) const; // Returns true when the requested mouse button is held down
        [[nodiscard]] bool mouse_down(int button) const; // Returns true on the frame that the requested mouse button is pressed
        [[nodiscard]] bool mouse_up(int button) const; // Returns true on the frame that the requested mouse button is released
        [[nodiscard]] float mouse_wheel() const;

    private:
        glm::vec2 m_window_pos = {};
        glm::vec2 m_mouse_pos_curr = {};
        glm::vec2 m_mouse_pos_prev = {};
        float m_mouse_wheel = 0.0f;
        float m_mouse_wheel_buffer = 0.0f;
        int m_mouse_button_curr[3] = {};
        int m_mouse_button_prev[3] = {};
        bool m_mouse_visible = true;
    };
}