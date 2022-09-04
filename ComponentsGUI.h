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

    struct SpriteRender {
        SpriteRender(const std::string& path, const int n_tex, const TextureType type) {
            tex_path = path;
            n_textures = n_tex;
            tex_type = type;
        }
        SpriteRender(const SpriteRender& other) {
            tex_path = std::string(other.tex_path);
            n_textures = other.n_textures;
            tex_type = other.tex_type;
        }
        std::string tex_path;
        int n_textures = 0; // Number of animation frames, sliced horizontally
        TextureType tex_type = TextureType::stretch;
    };

    struct Text {
        Text(const std::wstring& string, const glm::vec2 scl = { 2, 2 }, const glm::vec4 col = { 1, 1, 1, 1 }, const AnchorPoint ui_anchr = AnchorPoint::top_left, const AnchorPoint txt_anchr = AnchorPoint::top_left) {
            text = new wchar_t[string.size()+1];
            memcpy_s(text, (string.size() + 1) * sizeof(string[0]), string.data(), (string.size() + 1) * sizeof(string[0]));
            ui_anchor = txt_anchr;
            text_anchor = ui_anchr;
            color = col;
            scale = scl;
        }

        ~Text() {
            puts("~Text()");
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
    };

    struct Scrollable{}; // This is a tag without data
    struct NumberBox{}; // This is a tag without data
    struct Button{}; // This is a tag without data

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
        size_t index;
        VarType type;

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


    inline EntityID create_button(Scene& scene, const glm::vec2 top_left, const glm::vec2 bottom_right, std::function<void()> func, const float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left, const std::wstring& text = L"", glm::vec4 text_color = {1, 1, 1, 1}, glm::vec2 text_scale = {2, 2}) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, { top_left, bottom_right, depth, anchor });
        scene.add_component<Clickable>(entity, {std::move(func)});
        scene.add_component<MouseInteract>(entity);
        scene.add_component<SpriteRender>(entity, { "button.png", 1, TextureType::slice });
        scene.add_component<Text>(entity, { text, text_scale, text_color, AnchorPoint::center, AnchorPoint::center });
        scene.add_component<Button>(entity);
        return entity;
    }

    inline EntityID create_text(Scene& scene, const std::string& name, const glm::vec2 top_left, const glm::vec2 bottom_right, const std::wstring& text, glm::vec2 scale = { 1, 1 }, glm::vec4 color = { 1,1,1,1 }, const float depth = 0.0f, AnchorPoint ui_anchor_point = AnchorPoint::top_left, AnchorPoint text_anchor_point = AnchorPoint::top_left) {
        // Create entity and add components
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, { top_left, bottom_right, depth, ui_anchor_point });
        scene.add_component<Text>(entity, { text, scale, color, text_anchor_point });
        scene.add_component<Value>(entity, { name, VarType::wstring });

        // Bind the text string to the variable name
        const auto value_c = scene.get_component<Value>(entity);
        Value::bind(name, value_c->index);
        value_c->set_ptr(name, new std::wstring(L"TEST"));

        return entity;
    }

    inline EntityID create_numberbox(
        Scene& scene, 
        const std::string& name,
        const glm::vec2 top_left,
        const glm::vec2 bottom_right,
        const double min, 
        const double max, 
        const double step = 1.0,
        const double init_val = 0,
        const glm::vec4 text_color = {1, 1, 1, 1},
        const glm::vec2 text_scale = {2, 2},
        const float depth = 0.0f,
        AnchorPoint anchor = AnchorPoint::top_left, 
        AnchorPoint text_anchor_point = AnchorPoint::top_left
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, { top_left, bottom_right, depth, anchor });
        scene.add_component<Value>(entity, { name, VarType::float64 });
        scene.add_component<NumberRange>(entity, {min, max, step});
        scene.add_component<Draggable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<SpriteRender>(entity, { "numberbox.png", 1, TextureType::slice });
        scene.add_component<Text>(entity, { L"", text_scale, text_color, AnchorPoint::center, AnchorPoint::center });
        scene.get_component<Value>(entity)->set<double>(init_val);
        return entity;
    }

    inline void system_comp_sprite(Scene& scene, Renderer& renderer) {
        for (const auto entity : scene.view<Transform, SpriteRender>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* sprite = scene.get_component<SpriteRender>(entity);
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
            renderer.draw_box_textured(sprite->tex_path, sprite->tex_type, transform->top_left, transform->bottom_right, color, transform->depth + 0.001f, transform->anchor);
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
                            range->max == floor(range->max)
                            )
                            swprintf_s(text->text, 32, L"%.0f", val);
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

    inline void system_comp_mouse_interact(Scene& scene, Renderer& renderer, const Input& input) {
        // Loop over all MouseInteract components, and handle the state. In this loop we also handle click events since that's literally 2 extra lines of code
        for (const auto entity : scene.view<Transform, MouseInteract>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* clickable = scene.get_component<Clickable>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);

            const glm::vec2 anchor_offsets[] = {
                { 0,  0}, // center
                {-1,  1}, // top left
                { 0,  1}, // top
                { 1,  1}, // top right
                { 1,  0}, // right
                { 1, -1}, // bottom right
                { 0, -1}, // bottom
                {-1, -1}, // bottom left
                {-1,  0}, // left
            };

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
                // and the mouse is still on the component,
                if (is_inside_bb) {
                    // and the entity is clickable
                    if (clickable) {
                        // Call the function of this clickable
                        clickable->on_click();
                    }
                }
                // Reset the MouseInteract state back to idle
                mouse_interact->state = ClickState::idle;
            }
        }

        // Handle draggable components like sliders, numberboxes,
        for (const auto entity : scene.view<Value, Draggable, MouseInteract, NumberRange>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* value = scene.get_component<Value>(entity);
            auto* number_range = scene.get_component<NumberRange>(entity);

            // If the component is being dragged
            if (mouse_interact->state == ClickState::click) {
                // Get a reference to the value
                double& val = value->get_as_ref<double>();

                // Map the vertical mouse movement to the value
                val -= static_cast<double>(input.mouse_pos(MouseRelative::relative).y) * number_range->step;

                // Clamp the value to the bounds
                val = std::max(val, number_range->min);
                val = std::min(val, number_range->max);
            }
        }
    }

    inline void update_entities(Scene& scene, Renderer& renderer, const Input& input) {
        // Render sprites
        system_comp_sprite(scene, renderer);

        // Render text
        system_comp_text(scene, renderer);

        // Handle clickable components
        system_comp_mouse_interact(scene, renderer, input);
    }
}
