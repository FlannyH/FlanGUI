#include <chrono>
#include <iostream>

#include "ComponentsGUI.h"
#include "ComponentSystem.h"
#include "Input.h"
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
    Flan::Input input(renderer.window());

    // Create button
    Flan::create_button(scene, { { -100, -150 }, { 100, -250 }, 0.0f, Flan::AnchorPoint::center }, []()
        {
            printf("hi1!\n");
        }, { L"Button1", {2, 2},  { 1, 0, 0, 1 }, Flan::AnchorPoint::center, Flan::AnchorPoint::center});
    Flan::create_text(scene, "debug_text", { { 8, 8 }, {9, 9} }, { L"" });
    Flan::create_numberbox(scene, "debug_numberbox", { { 100, 600 }, { 200, 700 } }, { 0.0, 100.0, 1.0 }, 50.0);
    Flan::create_wheelknob(scene, "debug_numberbox", { { 300, 600 }, { 200, 700 } }, { 0.0, 100.0, 1.0 }, 50.0);

    float smooth_dt = 0.0f;
    [[maybe_unused]] float time = 0.0f;
    wchar_t frametime_text[256];

    while (!glfwWindowShouldClose(renderer.window())) {
        // Draw
        renderer.begin_frame();
        renderer.draw_circle_line({ 0,0 }, { 300, 300 }, { 0,1,0,1 }, 4, 0.1f, Flan::AnchorPoint::center);
        const float dt = calculate_delta_time();
        time += dt;
        smooth_dt = smooth_dt + (dt - smooth_dt) * (1.f-powf(0.02f, dt));
        swprintf_s(frametime_text, L"frametime: %.5f ms\nframe rate: %.3f fps\nmouse_pos_absolute: %.0f, %.0f\nmouse_pos_window: %.0f, %.0f\nmouse_pos_relative: %.0f, %.0f\nmouse_buttons = %i%i%i\nmouse_down = %i%i%i\nmouse_up = %i%i%i\nmouse_wheel = %.0f\ndebug_numberbox = %f",
            smooth_dt * 1000.f, 
            1.0f/smooth_dt, 
            input.mouse_pos(Flan::MouseRelative::absolute).x, 
            input.mouse_pos(Flan::MouseRelative::absolute).y, 
            input.mouse_pos(Flan::MouseRelative::window).x, 
            input.mouse_pos(Flan::MouseRelative::window).y,
            input.mouse_pos(Flan::MouseRelative::relative).x,
            input.mouse_pos(Flan::MouseRelative::relative).y,
            input.mouse_held(0), input.mouse_held(1), input.mouse_held(2),
            input.mouse_down(0), input.mouse_down(1), input.mouse_down(2),
            input.mouse_up(0), input.mouse_up(1), input.mouse_up(2),
            input.mouse_wheel(),
            Flan::Value::get<double>("debug_numberbox")
        );
        Flan::Value::set_ptr("debug_text", &frametime_text);
        Flan::update_entities(scene, renderer, input);

        renderer.end_frame();
        input.update(renderer.window());
    }
}
