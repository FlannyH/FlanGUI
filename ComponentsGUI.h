#pragma once
#include <functional>
#include <utility>

#include "ComponentSystem.h"
#include "Input.h"
#include "Renderer.h"
#include "glm/vec2.hpp"
//
//inline void* operator new(size_t size) {
//    void* ptr = malloc(size);
//    printf("allocated %i bytes at %p\n", size, ptr);
//    return ptr;
//}
//
//inline void operator delete(void* ptr) noexcept {
//    free(ptr);
//    printf("freed memory at %p\n", ptr);
//}

#define N_VALUES 256

namespace Flan {
    struct Transform {
        Transform(const glm::vec2 tl, const glm::vec2 br, const float dpth = 0.0f, const AnchorPoint anch = AnchorPoint::top_left) {
            const float x_min = std::min(tl.x, br.x);
            const float x_max = std::max(tl.x, br.x);
            const float y_min = std::min(tl.y, br.y);
            const float y_max = std::max(tl.y, br.y);
            top_left = {x_min, y_min};
            bottom_right = { x_max, y_max };
            anchor = anch;
            depth = dpth;
        }
        glm::vec2 top_left{}, bottom_right{};
        float depth{};
        AnchorPoint anchor{};
    };

    enum class ClickState {
        idle = 0,
        hover,
        click
    };

    struct Clickable {
        Clickable(std::function<void()> click) {
            on_click = std::move(click);
        }
        std::function<void()> on_click;
    };

    struct MouseInteract {
        ClickState state = ClickState::idle;
    };

    struct SpriteRender {};

    struct Sprite {
        Sprite(const std::string& path, const TextureType type = TextureType::stretch) {
            tex_path = path;
            tex_type = type;
        }
        Sprite(const Sprite& other) {
            tex_path = std::string(other.tex_path);
            tex_type = other.tex_type;
        }
        std::string tex_path;
        TextureType tex_type = TextureType::stretch;
    };

    struct Sprites {
        Sprites(std::initializer_list<Sprite> init_list) {
            // Insert the init_list's values
            sprites.insert(sprites.end(), init_list.begin(), init_list.end());
        }
        std::vector<Sprite> sprites;
    };

    struct Text {
        Text(const std::wstring& string = std::wstring(), const glm::vec2 scl = {2, 2}, const glm::vec4 col = {1, 1, 1, 1}, const AnchorPoint ui_anchr = AnchorPoint::top_left, const AnchorPoint txt_anchr = AnchorPoint::top_left) {
            text = new wchar_t[string.size()+1];
            memcpy_s(text, (string.size() + 1) * sizeof(string[0]), string.data(), (string.size() + 1) * sizeof(string[0]));
            ui_anchor = txt_anchr;
            text_anchor = ui_anchr;
            color = col;
            scale = scl;
        }

        ~Text() {
            delete[] text;
        }

        Text(const Text& other) {
            // Copy values
            auto tmp = std::wstring(other.text);
            text = new wchar_t[(tmp.size() + 1)];
            memcpy_s(text, (tmp.size() + 1) * 2, tmp.data(), (tmp.size() + 1) * 2);
            ui_anchor = other.ui_anchor;
            text_anchor = other.text_anchor;
            color = other.color;
            scale = other.scale;
        }

        wchar_t* text;
        AnchorPoint ui_anchor;
        AnchorPoint text_anchor;
        glm::vec4 color{};
        glm::vec2 scale{};
    };

    struct NumberRange {
        // Bounds - the value is stored inside a Value component
        double min;
        double max;
        double step;
    };
    
    struct Draggable {
        bool is_horizontal = false;
    };

    struct Hitbox {  
        // The minimum and maximum x and y coordinates of the hitbox, in pixels relative to the transform of the component.
        // Note that the y-coordinate of the top of the hitbox is 0 and it increases as it goes down.
        glm::vec2 top_left{}, bottom_right{};
        bool intersects(glm::vec2 pos) {
            return (
                pos.x >= top_left.x &&
                pos.x <= bottom_right.x &&
                pos.y >= top_left.y &&
                pos.y <= bottom_right.y
            );
        }
    };

    struct MultiHitbox {
        Hitbox hitboxes[16]{};
        size_t n_hitboxes;
    };

    struct RadioButton {
        std::vector<std::wstring> options;
        size_t current_selected_index = 0;
    };

    struct Scrollable{}; // This is a tag without data
    struct NumberBox{}; // This is a tag without data
    struct Button{}; // This is a tag without data
    struct WheelKnob{}; // This is a tag without data
    struct Slider{};

    inline static char* value_names[256]{};
    inline static uint64_t value_pool[256]{};
    inline static size_t value_pool_idx = 0;

    enum class VarType {
        none,
        wstring,
        float64
    };

    struct Value {
        // Index into value pool
        size_t index{};
        VarType type{};

        // Assign a new value index
        Value() {
            index = value_pool_idx++;
        }

        // Assign a new value index with a name
        Value(const std::string& name, const VarType var_type) {
            // If this name already exist, bind to that one
            index = get_index_from_name(name);
            type = var_type;
        }

        // Get variable index from name
        static size_t get_index_from_name(const std::string& name) {
            // If exists, return index
            for (int i = 0; i < N_VALUES; i++) {
                if (value_names[i] != nullptr && name == value_names[i]) {
                    return i;
                }
            }
            // Otherwise create new variable
            value_names[value_pool_idx] = new char[name.size() + 1];
            strcpy_s(value_names[value_pool_idx], name.size() + 1, name.c_str());
            value_pool[value_pool_idx] = 0;
            return value_pool_idx++;
        }

        // Get the current value
        template<typename T>
        T& get_as_ref() {
            static_assert(sizeof(T) <= sizeof(value_pool[0]));
            return (T&)(value_pool[index]);
        }
        template<typename T>
        T* get_as_ptr() {
            return reinterpret_cast<T*>(value_pool[index]);
        }

        // Get value from index
        template<typename T>
        static T& get(const size_t index) {
            static_assert(sizeof(T) <= sizeof(value_pool[0]));
            return (T&)(value_pool[index]);
        }

        // Get value from name
        template<typename T>
        static T& get(const std::string& name) {
            static_assert(sizeof(T) <= sizeof(value_pool[0]));
            return (T&)(value_pool[get_index_from_name(name)]);
        }

        // Set the current value
        template<typename T>
        void set(T value) {
            static_assert(sizeof(T) <= sizeof(value_pool[0]));
            get_as_ref<T>() = (T)value;
        }

        // Set the current value
        template<typename T>
        static void set_value(const std::string& name, T value) {
            static_assert(sizeof(T) <= sizeof(value_pool[0]));
            value_pool[get_index_from_name(name)] = value;
        }

        // Set the current pointer
        template<typename T>
        static void set_ptr(const std::string& name, T* value) {
            value_pool[get_index_from_name(name)] = reinterpret_cast<uint64_t>(value);
        }

        // Bind a name to a value
        static void bind(const std::string& name, const size_t index) {
            value_names[index] = new char[name.size() + 1];
            strcpy_s(value_names[index], name.size() + 1, name.c_str());
        }
    };


    inline EntityID create_button(Scene& scene, 
        const Transform& transform,
        std::function<void()> func,
        const Text& text = { L"", {2, 2}, {1, 1, 1, 1}, AnchorPoint::center, AnchorPoint::center }
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Clickable>(entity, {std::move(func)});
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Sprites>(entity, { {"button.png", TextureType::slice} });
        scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.add_component<Button>(entity);
        return entity;
    }

    inline EntityID create_text(Scene& scene,
        const std::string& name,
        const Transform& transform,
        Text&& text
    ) {
        // Create entity and add components
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Text>(entity, std::move(text));
        scene.add_component<Value>(entity, { name, VarType::wstring });

        // Bind the text string to the variable name
        const auto value_c = scene.get_component<Value>(entity);
        Value::bind(name, value_c->index);
        Value::set_ptr(name, new std::wstring(L"TEST"));

        return entity;
    }

    inline EntityID create_numberbox(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0 },
        const double init_val = 0,
        const Text& text = { L"",{2, 2}, {1,1,1,1}, AnchorPoint::center, AnchorPoint::center, }
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 });
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity);
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Sprites>(entity, { {"numberbox.png", TextureType::slice} });
        scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(init_val);
        return entity;
    }

    inline EntityID create_wheelknob(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0 },
        const double init_val = 0,
        const Text& text = { L"",{2, 2}, {1,0,1,1}, AnchorPoint::bottom, AnchorPoint::center, }
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 });
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity);
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<WheelKnob>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(init_val);
        return entity;
    }

    inline EntityID create_slider(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0 },
        const double init_val = 0,
        Text text = { L"",{2, 2}, {1,1,1,1}, AnchorPoint::center, AnchorPoint::bottom, }
    ) {
        const glm::vec2 scale = transform.bottom_right - transform.top_left;
        const bool is_horizontal = (scale.x) > (scale.y);
        if (is_horizontal) {
            text.ui_anchor = AnchorPoint::center;
            text.text_anchor = AnchorPoint::top;
        }

        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 });
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity, { is_horizontal });
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Slider>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(init_val);
        return entity;
    }

    inline EntityID create_radio_button(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        std::vector<std::wstring> options,
        const size_t initial_index = 0
    )
    {
        // Create hitboxes for the radio buttons
        MultiHitbox multihitbox{};
        float curr_y = 0;
        float delta_y = (transform.bottom_right.y - transform.top_left.y) / options.size();

        // Populate hitboxes
        for (size_t i = 0; i < options.size(); ++i) {
            // Hitboxes
            multihitbox.hitboxes[i].top_left.x = 0;
            multihitbox.hitboxes[i].bottom_right.x = transform.bottom_right.x - transform.top_left.x;
            multihitbox.hitboxes[i].top_left.y = curr_y;
            multihitbox.hitboxes[i].bottom_right.y = curr_y + delta_y;
            curr_y += delta_y;
        }
        multihitbox.n_hitboxes = options.size();

        // Create entity
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 }); //todo: make add int type
        scene.add_component<MultiHitbox>(entity, multihitbox);
        scene.add_component<RadioButton>(entity, {options, initial_index});
        scene.add_component<MouseInteract>(entity);
        return entity;
    }

    inline void system_comp_sprite(Scene& scene, Renderer& renderer) {
        for (const auto entity : scene.view<Transform, Sprites, SpriteRender>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* sprite = scene.get_component<Sprites>(entity);
            // If it has a clickable component, use that to render the button
            glm::vec4 color = { 1, 1, 1, 1 };
            if (const auto* mouse_interact = scene.get_component<MouseInteract>(entity)) {
                if (mouse_interact->state == ClickState::hover) {
                    color *= 0.9f;
                }
                if (mouse_interact->state == ClickState::click) {
                    color *= 0.7f;
                }
            }
            renderer.draw_box_textured(sprite->sprites[0].tex_path, sprite->sprites[0].tex_type, transform->top_left, transform->bottom_right, color, transform->depth + 0.001f, transform->anchor);
        }
    }

    inline void system_comp_text(Scene& scene, Renderer& renderer) {
        for (const auto entity : scene.view<Transform, Text>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* text = scene.get_component<Text>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* range = scene.get_component<NumberRange>(entity);
            const glm::vec2 anchor_offsets[] = {
                {0.5f, 0.5f}, // center
                {0.0f, 0.0f}, // top left
                {0.5f, 0.0f}, // top
                {1.0f, 0.0f}, // top right
                {1.0f, 0.5f}, // right
                {1.0f, 1.0f}, // bottom right
                {0.5f, 1.0f}, // bottom
                {0.0f, 1.0f}, // bottom left
                {0.0f, 0.5f}, // left
            };

            if (value) {
                if (value->type == VarType::wstring) {
                    const auto string = (value->get_as_ptr<wchar_t>());
                    text->text = string;
                }
                if (value->type == VarType::float64) {
                    double& val = Value::get<double>(value->index);
                    delete text->text;
                    text->text = new wchar_t[32];

                    //If all parts of the range are a whole number, print as if it were an integer
                    if (range) {
                        if (range->step == floor(range->step) &&
                            range->min == floor(range->min) &&
                            range->max == floor(range->max)) 
                        {
                            swprintf_s(text->text, 32, L"%.0f", val);
                        }
                    }
                    else {
                        swprintf_s(text->text, 32, L"%.2f", val);
                    }
                }
            }

            // Calculate position relative to top_left
            const glm::vec2 top_left = transform->top_left + glm::vec2(renderer.resolution()) * anchor_offsets[static_cast<size_t>(transform->anchor)];
            const glm::vec2 offset_from_top_left = (transform->bottom_right - transform->top_left) * anchor_offsets[static_cast<size_t>(text->ui_anchor)];
            renderer.draw_text(text->text, top_left + offset_from_top_left, text->scale, text->color, transform->depth, AnchorPoint::top_left, text->text_anchor);
            renderer.draw_circle_solid(top_left, { 4,4 }, { 1,0,1,1 });
        }
    }

    inline void system_comp_special_render(Scene& scene, Renderer& renderer) {
        // Wheel knobs
        for (const auto entity : scene.view<Transform, Value, NumberRange, WheelKnob>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* range = scene.get_component<NumberRange>(entity);

            // Draw the wheel
            const glm::vec2 center = (transform->top_left + transform->bottom_right) / 2.0f;
            const glm::vec2 scale = center - transform->top_left;
            double& val = value->get_as_ref<double>();
            const float angle = static_cast<float>(1.5 * 3.14159265359 + ((val - range->min) / (range->max - range->min) - 0.5) * (1.75 * 3.14159265359));
            const glm::vec2 line_b = center + glm::vec2(cosf(angle), sinf(angle)) * scale;
            renderer.draw_circle_solid(center, scale, { 1, 1, 1, 1 }, transform->depth + 0.0002f, transform->anchor);
            renderer.draw_circle_line(center, scale, { 0, 0, 0, 1 }, 2, transform->depth + 0.0001f, transform->anchor);
            renderer.draw_line(center, line_b, { 0, 0, 0, 1 }, 4, transform->depth + 0.0001f);
        }

        // Sliders
        for (const auto entity : scene.view<Transform, Value, NumberRange, Slider>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* range = scene.get_component<NumberRange>(entity);
            auto* text = scene.get_component<Text>(entity);
            auto* draggable = scene.get_component<Draggable>(entity);

            // Draw the slider
            glm::vec2 bottom_right = transform->bottom_right;
            if (text && draggable && draggable->is_horizontal == false) {
                bottom_right.y -= renderer.get_font_height() * text->scale.y;
            }
            const glm::vec2 center = (transform->top_left + bottom_right) / 2.0f;
            const glm::vec2 scale = center - transform->top_left;
            double& val = value->get_as_ref<double>();
            if (draggable && draggable->is_horizontal)
            {
                const float dist_left = static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * scale.x;
                renderer.draw_line(center + glm::vec2{ scale.x, 0 }, center - glm::vec2{ scale.x, 0 }, { 0, 0, 0, 1 }, 4, transform->depth + 0.0002f);
                renderer.draw_line(center + glm::vec2{ scale.x, 0 }, center - glm::vec2{ scale.x, 0 }, { 1, 1, 1, 1 }, 2, transform->depth + 0.0001f);
                renderer.draw_box_solid(center + glm::vec2{ dist_left - 10, -20 }, center + glm::vec2{ dist_left + 10, +20 }, { 1,1,1,1 });
            }
            else {
                const float dist_bottom = static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * -scale.y;
                renderer.draw_line(center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y}, { 0, 0, 0, 1 }, 4, transform->depth + 0.0002f);
                renderer.draw_line(center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y}, { 1, 1, 1, 1 }, 2, transform->depth + 0.0001f);
                renderer.draw_box_solid(center + glm::vec2{ -20, dist_bottom - 10 }, center + glm::vec2{ +20, dist_bottom + 10 }, { 1,1,1,1 });
            }
        }

        // Radio buttons
        for (const auto entity : scene.view<Transform, Value, RadioButton>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* radio_button = scene.get_component<RadioButton>(entity);

            // Get some information ready for the sake of my mental sanity in writing this code
            size_t n_options = radio_button->options.size();
            float vertical_spacing = (transform->bottom_right.y - transform->top_left.y) / n_options;
            float margin = 2.0f;
            float circle_size_max = vertical_spacing / 2.0f - margin;
            glm::vec2 circle_base_offset = transform->top_left + glm::vec2(margin + circle_size_max);
            float outline_circle_radius = 20;
            float selected_circle_radius = 14;
            float text_margin = 20;
            
            // Display every option
            for (size_t i = 0; i < n_options; ++i) {
                // Determine a nice color based on what the mouse is doing
                glm::vec4 color = { 1, 1, 1, 1 };
                const auto* mouse_interact = scene.get_component<MouseInteract>(entity);
                if (mouse_interact != nullptr && i == radio_button->current_selected_index) {
                    if (mouse_interact->state == ClickState::hover) {
                        color *= 0.9f;
                    }
                    if (mouse_interact->state == ClickState::click) {
                        color *= 0.7f;
                    }
                }

                // Draw the circle outline for each of them
                renderer.draw_circle_line(circle_base_offset + glm::vec2(0, vertical_spacing * i), glm::vec2(outline_circle_radius), color, 2.0f, transform->depth, transform->anchor);

                // Draw the text
                renderer.draw_text(radio_button->options[i], circle_base_offset + glm::vec2(0, vertical_spacing * i) + glm::vec2(outline_circle_radius + text_margin, 0), { 2, 2 }, color, transform->depth, AnchorPoint::top_left, AnchorPoint::left);

                // Draw selected circle
                if (i == radio_button->current_selected_index) {
                    renderer.draw_circle_solid(circle_base_offset + glm::vec2(0, vertical_spacing * i), glm::vec2(selected_circle_radius), color, transform->depth, transform->anchor);
                }
            }
        }
    }
    
    inline void system_comp_mouse_interact(Scene& scene, Renderer& renderer, Input& input) {
        // Loop over all MouseInteract components, and handle the state. In this loop we also handle click events since that's literally 2 extra lines of code
        for (const auto entity : scene.view<Transform, MouseInteract>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* clickable = scene.get_component<Clickable>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);

            // Get mouse position, and get an actual correct top-left and bottom-right
            const glm::vec2 mouse_pos = renderer.pixels_to_normalized(input.mouse_pos(MouseRelative::window), AnchorPoint::top_left);
            glm::vec2 tl_ = renderer.pixels_to_normalized(transform->top_left, transform->anchor);
            glm::vec2 br_ = renderer.pixels_to_normalized(transform->bottom_right, transform->anchor);
            const glm::vec2 tl = { std::min(tl_.x, br_.x), std::min(tl_.y, br_.y) };
            const glm::vec2 br = { std::max(tl_.x, br_.x), std::max(tl_.y, br_.y) };

            // Determine whether the mouse is inside the component's bounding box
            const bool is_inside_bb =
                mouse_pos.x >= tl.x &&
                mouse_pos.y >= tl.y &&
                mouse_pos.x <= br.x &&
                mouse_pos.y <= br.y;

            // If the element hasn't been clicked
            if (mouse_interact->state != ClickState::click) {
                // If the mouse cursor is inside the UI component's bounding box, set the ClickState to hover
                if (is_inside_bb) {
                    mouse_interact->state = ClickState::hover;
                }
                // Otherwise set it to idle
                else {
                    mouse_interact->state = ClickState::idle;
                }
            }
            // If we're hovering over the element and we click, set the ClickState to cliking
            if (input.mouse_down(0)) {
                if (mouse_interact->state == ClickState::hover) {
                    mouse_interact->state = ClickState::click;
                }
            }
            // If we release the mouse while this element is in click state,
            if (input.mouse_up(0) && mouse_interact->state == ClickState::click) {
                // and the mouse is still on the component, and the component is clickable
                if (is_inside_bb && clickable) {
                    // Call the function of this clickable
                    clickable->on_click();
                }
                // Reset the MouseInteract state back to idle
                mouse_interact->state = ClickState::idle;
                input.mouse_visible(true);
            }
        }

        // Handle draggable components like sliders, numberboxes,
        for (const auto entity : scene.view<Value, Draggable, MouseInteract, NumberRange>()) {
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* number_range = scene.get_component<NumberRange>(entity);
            auto* draggable = scene.get_component<Draggable>(entity);

            // If the component is being dragged
            if (mouse_interact->state == ClickState::click) {
                // Get a reference to the value
                double& val = value->get_as_ref<double>();

                // Map the mouse movement to the value
                if (draggable && draggable->is_horizontal) {
                    val += static_cast<double>(input.mouse_pos(MouseRelative::relative).x) * number_range->step;
                }
                else {
                    val -= static_cast<double>(input.mouse_pos(MouseRelative::relative).y) * number_range->step;
                }

                // Clamp the value to the bounds
                val = std::max(val, number_range->min);
                val = std::min(val, number_range->max);

                // Make the mouse invisible
                input.mouse_visible(false);
            }
        }

        // Handle scrollable components like sliders, numberboxes, listboxes
        for (const auto entity : scene.view<Value, Scrollable, MouseInteract, NumberRange>()) {
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* number_range = scene.get_component<NumberRange>(entity);

            // If the component is hovered over
            if (mouse_interact->state == ClickState::hover) {
                // Get a reference to the value
                double& val = value->get_as_ref<double>();

                // Map the vertical mouse scroll to the value
                val += static_cast<double>(input.mouse_wheel()) * number_range->step;

                // Clamp the value to the bounds
                val = std::max(val, number_range->min);
                val = std::min(val, number_range->max);
            }
        }
    }

    inline void system_comp_radio_buttons(Scene& scene, Renderer& renderer, Input& input) {
        // Handle scrollable components like sliders, numberboxes, listboxes
        for (const auto entity : scene.view<Transform, Value, RadioButton, MultiHitbox>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* radio_button = scene.get_component<RadioButton>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);

            // Make sure it also has a mouse interact component
            if (mouse_interact == nullptr) {
                continue;
            }

            // If the component is clicked on in general
            if (input.mouse_down(0) && mouse_interact->state == ClickState::click) {
                // Check for each hitbox
                for (size_t i = 0; i < multi_hitbox->n_hitboxes; ++i) {
                    // Transform the hitbox from local space to window space
                    Hitbox hitbox = multi_hitbox->hitboxes[i];
                    hitbox.top_left += renderer.apply_anchor_in_pixel_space(transform->top_left, transform->anchor);
                    hitbox.bottom_right += renderer.apply_anchor_in_pixel_space(transform->top_left, transform->anchor);

                    // See if it intersects
                    if (hitbox.intersects(input.mouse_pos(MouseRelative::window)))
                    {
                        // If so, select that value
                        radio_button->current_selected_index = i;
                        value->get_as_ref<double>() = static_cast<double>(i);
                    }
                }
            }
        }
    }

    inline void update_entities(Scene& scene, Renderer& renderer, Input& input) {
        // Render sprites
        system_comp_sprite(scene, renderer);
        system_comp_special_render(scene, renderer);

        // Render text
        system_comp_text(scene, renderer);

        // Handle clickable components
        system_comp_mouse_interact(scene, renderer, input);

        // Handle radio buttons
        system_comp_radio_buttons(scene, renderer, input);

        // Debug
        for (const auto entity : scene.view<Transform>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            renderer.draw_box_line(transform->top_left, transform->bottom_right, { 1, 0, 1, 1 }, 1, 0, transform->anchor);
        }
    }
}
