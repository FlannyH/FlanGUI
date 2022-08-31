#include <chrono>
#include <iostream>

#include "ComponentsGUI.h"
#include "ComponentSystem.h"
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

struct Transform {
    glm::vec2 pos;
};

int main()
{
    Flan::Renderer renderer;
    Flan::Scene scene;
    renderer.init();

    // Create button
    auto botton1 = Flan::button(scene, { 0, -100 }, { 100, -200 }, [&]()
        {
            printf("hi1!\n");
        }, 0.0f, Flan::AnchorPoint::center);
    auto botton2 = Flan::button(scene, { 0, 0 }, { 100, -100 }, [&]()
        {
            printf("hi2!\n");
        }, 0.0f, Flan::AnchorPoint::center);
    auto botton3 = Flan::button(scene, { 0, 100 }, { 100, 0 }, [&]()
        {
            printf("hi3!\n");
        }, 0.0f, Flan::AnchorPoint::center);

    float smooth_dt = 0.0f;
    float time = 0.0f;

    while (!glfwWindowShouldClose(renderer.window())) {
        // Draw
        renderer.begin_frame();
        Flan::update_entities(scene, renderer);
        renderer.draw_circle_line({ 0,0 }, { 600, 600 }, { 0,1,0,1 }, 4, 0.1f, Flan::AnchorPoint::center);
        //renderer.draw_circle_solid({ 0,0 }, { 580, 580 }, { 0,0.2,0,1 }, 0.1f, Flan::AnchorPoint::center);
        wchar_t frametime_text[64];
        const float dt = calculate_delta_time();
        time += dt;
        smooth_dt = smooth_dt + (dt - smooth_dt) * 0.0005f;
        swprintf_s(frametime_text, L"frametime: %.5f ms\nframe rate: %.3f fps", smooth_dt * 1000.f, 1.0f/smooth_dt);
        renderer.draw_text(frametime_text, { 16, -32 }, { 4.f, 4.f }, { 1.0, 0.7, 1, 0 }, 0.0f);
        //renderer.draw_box_line({ -400, 400 }, { 400, -400 }, { 1, 0, 0, 1 }, 8, 0.0f, Flan::AnchorPoint::center);
        //renderer.draw_box_solid({ -380, 380 }, { 380, -380 }, { 0.1, 0, 0, 1 }, 0.1f, Flan::AnchorPoint::center);
        //renderer.draw_box_textured("test.png", { -350, 350 }, {350, -350}, {1, 0.5f, 1, 1}, 8, 0.0f, Flan::AnchorPoint::center);

        // Draw bubbles
        renderer.draw_circle_line({ 100.f * sinf(time * 0.3f) - 500.f, fmod(time * 1000.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 }, 1.0f, 0.0f, Flan::AnchorPoint::center);
        renderer.draw_circle_line({  75.f * sinf(time * 0.9f) + 200.f, fmod(time * 700.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 }, 1.0f, 0.0f, Flan::AnchorPoint::center);
        renderer.draw_circle_line({ 135.f * sinf(time * 1.2f) - 400.f, fmod(time * 800.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 }, 1.0f, 0.0f, Flan::AnchorPoint::center);
        renderer.draw_circle_line({  54.f * sinf(time * 0.5f) + 300.f, fmod(time * 1200.f, 2000.f) - 1000.f }, { 50, 50 }, { 1, 1, 1, 1 }, 1.0f, 0.0f, Flan::AnchorPoint::center);

        renderer.end_frame();
    }
}
