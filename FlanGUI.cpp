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
    //auto botton1 = Flan::button(scene, { 0, -100 }, { 100, -200 }, []()
    //    {
    //        printf("hi1!\n");
    //    }, 0.0f, Flan::AnchorPoint::center);
    //auto botton2 = Flan::button(scene, { 0, 0 }, { 100, -100 }, []()
    //    {
    //        printf("hi2!\n");
    //    }, 0.0f, Flan::AnchorPoint::center);
    //auto botton3 = Flan::button(scene, { 0, 100 }, { 100, 0 }, []()
    //    {
    //        printf("hi3!\n");
    //    }, 0.0f, Flan::AnchorPoint::center);

    float smooth_dt = 0.0f;
    float time = 0.0f;

    while (!glfwWindowShouldClose(renderer.window())) {
        // Draw
        renderer.begin_frame();
        Flan::update_entities(scene, renderer, input);
        renderer.draw_circle_line({ 0,0 }, { 300, 300 }, { 0,1,0,1 }, 4, 0.1f, Flan::AnchorPoint::center);
        //renderer.draw_circle_solid({ 0,0 }, { 580, 580 }, { 0,0.2,0,1 }, 0.1f, Flan::AnchorPoint::center);
        wchar_t frametime_text[256];
        const float dt = calculate_delta_time();
        time += dt;
        smooth_dt = smooth_dt + (dt - smooth_dt) * 0.01f;
        swprintf_s(frametime_text, L"frametime: %.5f ms\nframe rate: %.3f fps\nmouse_pos_absolute: %.0f, %.0f\nmouse_pos_window: %.0f, %.0f\nmouse_pos_relative: %.0f, %.0f\nmouse_buttons = %i%i%i\nmouse_down = %i%i%i\nmouse_up = %i%i%i\nmouse_wheel = %.0f", 
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
            input.mouse_wheel()
        );
        renderer.draw_text(frametime_text, { 0, 0 }, { 2.f, 2.f }, { 1.0, 0.7, 1, 0 }, 0.0f, Flan::AnchorPoint::center, (Flan::AnchorPoint)((int)time % 9));
        renderer.draw_line({ -400, 0 }, { 400, 0 }, { 1,1,1,1 }, 1, 0, Flan::AnchorPoint::center);
        renderer.draw_line({ 0, -400 }, { 0, 400 }, { 1,1,1,1 }, 1, 0, Flan::AnchorPoint::center);
        //renderer.draw_box_line({ -400, 400 }, { 400, -400 }, { 1, 0, 0, 1 }, 8, 0.0f, Flan::AnchorPoint::center);
        //renderer.draw_box_solid({ -380, 380 }, { 380, -380 }, { 0.1, 0, 0, 1 }, 0.1f, Flan::AnchorPoint::center);
        //renderer.draw_box_textured("test.png", { -350, 350 }, {350, -350}, {1, 0.5f, 1, 1}, 8, 0.0f, Flan::AnchorPoint::center);

        renderer.end_frame();
        input.update(renderer.window());
    }
}
