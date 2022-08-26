#include <iostream>

#include "Renderer.h"

int main()
{
    Flan::Renderer renderer;
    renderer.init();
    float t = 0.0f;
    while (!glfwWindowShouldClose(renderer._window)) {
        t += 0.0005f;
        renderer.begin_frame();
        renderer.draw_line({ 0.0f, 0.0f }, { 500 * sinf(t), 500 * cosf(t)}, {1.0f, 0.0f, 1.0f, 0.0f}, 20.f, 0.0f);
        renderer.draw_linebox({ -500, 500 }, { 500, -500 }, { 0, 1, 0, 1 }, 5, 0.0f);
        renderer.draw_line({-600, -600}, {-700, -700}, {1, 0, 0,1}, 3, 0);
        renderer.draw_line({ -600, -600 }, { -600, -700 }, { 1, 0, 0,1 }, 3, 0);
        renderer.draw_linecircle({ 0, 0 }, { 500, 500 }, { 0, 1, 1, 1 }, 10);
        renderer.end_frame();
    }
}
