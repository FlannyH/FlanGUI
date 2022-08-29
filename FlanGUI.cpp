#include <chrono>
#include <iostream>

#include "Renderer.h"

static std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
static std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();

static float calculate_delta_time()
{
    end = std::chrono::steady_clock::now();
    const std::chrono::duration<float> delta = end - start;
    start = std::chrono::steady_clock::now();
    return delta.count();
}


int main()
{
    Flan::Renderer renderer;
    renderer.init();
    float smooth_dt = 0.0f;
    float time = 0.0f;
    while (!glfwWindowShouldClose(renderer._window)) {
        // Draw
        renderer.begin_frame();
        renderer.draw_linecircle({ 0,0 }, { 600, 600 }, { 0,1,0,1 }, 4, 0.1f);
        wchar_t frametime_text[64];
        float dt = calculate_delta_time();
        time += dt;
        smooth_dt = smooth_dt + (dt - smooth_dt) * 0.0005f;
        swprintf_s(frametime_text, L"frametime: %.5f ms\nframe rate: %.3f fps", smooth_dt * 1000.f, 1.0f/smooth_dt);
        renderer.draw_text(frametime_text, { -1240, 680 }, { 4.f, 4.f }, { 1.0, 0.7, 1, 0 }, 0.0f);
        renderer.draw_linebox({ -400, 400 }, { 400, -400 }, { 1, 0, 0, 1 }, 8);
        renderer.draw_solidbox({ -350, 350 }, { 350, -350 }, { 0.5f, 0, 0, 1 }, 8);

        // Draw bubbles

        renderer.draw_linecircle({ 100.f * sinf(time * 0.3f) - 500.f, fmod(time * 1000.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 });
        renderer.draw_linecircle({  75.f * sinf(time * 0.9f) + 200.f, fmod(time * 700.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 });
        renderer.draw_linecircle({ 135.f * sinf(time * 1.2f) - 400.f, fmod(time * 800.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 });
        renderer.draw_linecircle({  54.f * sinf(time * 0.5f) + 300.f, fmod(time * 1200.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 });

        renderer.end_frame();
    }
}
