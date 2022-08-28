#include <iostream>

#include "Renderer.h"

int main()
{
    Flan::Renderer renderer;
    renderer.init();
    while (!glfwWindowShouldClose(renderer._window)) {
        // Get time
        std::time_t t = std::time(0);
        std::tm now;
        localtime_s(&now, &t);
        float sec = now.tm_sec;
        float min = now.tm_min + sec / 60.f;
        float hour = now.tm_hour + min / 60.f;
        float sec_angle  = -((float)sec / 60.f) * 2.f * 3.14159265359f + (0.5f * 3.14159265359f);
        float min_angle  = -(float)min / 60.f * 2.f * 3.14159265359f + (0.5f * 3.14159265359f);
        float hour_angle = -(float)hour / 12.f * 2.f * 3.14159265359f + (0.5f * 3.14159265359f);

        // Draw circle
        renderer.begin_frame();
        renderer.draw_linecircle({ 0,0 }, { 600, 600 }, { 0,1,0,1 }, 4);
        renderer.draw_linecircle({ 0,0 }, { 20, 20 }, { 0,1,0,1 }, 10);

        // Draw hour stripes
        for (int i = 0; i < 12; i++) {
            glm::vec2 direction = {
                cosf(float(i) / 12 * 2 * 3.14159265359f),
                sinf(float(i) / 12 * 2 * 3.14159265359f),
            };
            renderer.draw_line(direction * 500.f, direction * 600.f, { 0, 0.5f, 0, 2 });
        }

        // Draw minute/second stripes
        for (int i = 0; i < 60; i++) {
            glm::vec2 direction = {
                cosf(float(i) / 60 * 2 * 3.14159265359f),
                sinf(float(i) / 60 * 2 * 3.14159265359f),
            };
            renderer.draw_line(direction * 560.f, direction * 600.f, { 0, 0.5f, 0, 1 });
        }

        //Draw second hand
        {
            glm::vec2 direction = {
                cosf(sec_angle),
                sinf(sec_angle),
            };
            renderer.draw_line({ 0,0 }, direction * 520.f, { 0, 1, 0, 1 }, 2);
        }
        //Draw minute hand
        {
            glm::vec2 direction = {
                cosf(min_angle),
                sinf(min_angle),
            };
            renderer.draw_line({ 0,0 }, direction * 470.f, { 0, 1, 0, 1 }, 8);
        }
        //Draw hour hand
        {
            glm::vec2 direction = {
                cosf(hour_angle),
                sinf(hour_angle),
            };
            renderer.draw_line({ 0,0 }, direction * 250.f, { 0, 1, 0, 1 }, 10);
        }

        renderer.end_frame();
    }
}
