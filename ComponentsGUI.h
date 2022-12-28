#pragma once
#include <functional>
#include <utility>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "ComponentSystem.h"
#include "Input.h"
#include "Renderer.h"
#include "glm/vec2.hpp"
#include "CommonStructs.h"

#ifdef _DEBUG
static int chunks_allocated;
static int chunks_deallocated;

inline void* operator new(size_t size) {
    void* ptr = malloc(size);
    //printf("allocated %i bytes at %p\n", size, ptr);
    chunks_allocated++;
    printf("total chunks allocated: %i\t\t\r", chunks_allocated - chunks_deallocated);
    return ptr;
}

inline void operator delete(void* ptr) noexcept {
    free(ptr);
    //printf("freed memory at %p\n", ptr);
    chunks_deallocated++;
    printf("total chunks allocated: %i\t\t\r", chunks_allocated - chunks_deallocated);
}
#endif

#define N_VALUES 256

namespace Flan {
    template <typename T>
    T sign(T v) {
        return (v > 0) ? +1 : -1;
    }

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
            text_length = string.size() + 1;
            text = new wchar_t[text_length];
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
            text_length = tmp.size() + 1;
            text = new wchar_t[text_length];
            memcpy_s(text, text_length * 2, tmp.data(), text_length * 2);
            ui_anchor = other.ui_anchor;
            text_anchor = other.text_anchor;
            color = other.color;
            scale = other.scale;
        }

        wchar_t* text;
        size_t text_length;
        AnchorPoint ui_anchor;
        AnchorPoint text_anchor;
        glm::vec4 color{};
        glm::vec2 scale{};
        glm::vec2 margins = { 8, 8 };
    };

    struct NumberRange {
        // Bounds - the value is stored inside a Value component
        double min;
        double max;
        double step;
        double default_value = 0.0;
        uint32_t visual_decimal_places = 2;
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
        ClickState click_states[16]{};
        size_t n_hitboxes;
    };

    struct RadioButton {
        std::vector<std::wstring> options;
        size_t current_selected_index = 0;
    };

    struct Combobox {
        std::vector<std::wstring> list_items;
        float button_height = 60.0f;
        float list_height = 360.0f;
        float item_height = 60.0f;
        float target_scroll_position = 0.0f;
        float current_scroll_position = 0.0f;
        bool is_list_open = false;
        int current_selected_index = 0;
    };

    struct Scrollable{}; // This is a tag without data
    struct NumberBox{}; // This is a tag without data
    struct Button{}; // This is a tag without data
    struct WheelKnob{}; // This is a tag without data
    struct Slider{};
    struct Box {
        glm::vec4 color_inner = { 0.25f, 0.25f, 0.25f, 1.0f };
        glm::vec4 color_outer = { 1.0f, 1.0f, 1.0f, 1.0f };
        float thickness = 2.0f;
    };

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
        //scene.add_component<Sprites>(entity, { {"button.png", TextureType::slice} });
        //scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.add_component<Button>(entity);
        scene.add_component<Box>(entity, { {0.7f, 0.7f, 0.7f, 0.7f}, {1, 1, 1, 1}, 2.0f });
        return entity;
    }

    inline EntityID create_text(Scene& scene,
        const std::string& name,
        const Transform& transform,
        Text&& text,
        bool has_box = false
    ) {
        // Create entity and add components
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Text>(entity, std::move(text));
        scene.add_component<Value>(entity, { name, VarType::wstring });
        if (has_box)
            scene.add_component<Box>(entity);

        // Bind the text string to the variable name
        const auto value_c = scene.get_component<Value>(entity);
        wchar_t* text_to_put = new wchar_t[text.text_length + 1];
        memcpy_s(text_to_put, (text.text_length + 1) * 2, text.text, (text.text_length + 1) * 2);
        Value::bind(name, value_c->index);
        Value::set_ptr(name, text_to_put);

        return entity;
    }

    inline EntityID create_numberbox(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0, 0.0, 0 },
        const Text& text = { L"",{2, 2}, {1,1,1,1}, AnchorPoint::center, AnchorPoint::center, }
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 });
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity);
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Box>(entity);
        //scene.add_component<Sprites>(entity, { {"numberbox.png", TextureType::slice} });
        //scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(range.default_value);
        return entity;
    }

    inline EntityID create_wheelknob(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0, 0.0, 0 },
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
        scene.get_component<Value>(entity)->set<double>(range.default_value);
        return entity;
    }

    inline EntityID create_slider(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const NumberRange& range = { 0.0, 100.0, 1.0, 0.0, 0 },
        bool has_text = true,
        Text text = { L"",{2, 2}, {1,1,1,1}, AnchorPoint::center, AnchorPoint::bottom, }
    ) {
        const glm::vec2 scale = transform.bottom_right - transform.top_left;
        const bool is_horizontal = (scale.x) > (scale.y);
        if (is_horizontal) {
            text.ui_anchor = AnchorPoint::bottom;
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
        if (has_text) scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(range.default_value);
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

    inline EntityID create_combobox(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        std::vector<std::wstring> items,
        const size_t initial_index = 0,
        const float item_height = 40.0f,
        const float list_height = 420.0f
    )
    {
        // Create hitboxes
        MultiHitbox multi_hitbox{};
        multi_hitbox.hitboxes[0].top_left = { 0, 0 };
        multi_hitbox.hitboxes[0].bottom_right = transform.bottom_right - transform.top_left;
        multi_hitbox.hitboxes[1].top_left = { 0, transform.bottom_right.y - transform.top_left.y };
        multi_hitbox.hitboxes[1].bottom_right = { transform.bottom_right.x - transform.top_left.x, transform.bottom_right.y + list_height };
        multi_hitbox.n_hitboxes = 2;

        // Create combobox component
        Combobox combobox{};
        combobox.is_list_open = false;
        combobox.item_height = item_height;
        combobox.list_height = list_height;
        combobox.current_selected_index = initial_index;
        combobox.current_scroll_position = 0.0f;
        combobox.list_items = items;
        combobox.button_height = transform.bottom_right.y - transform.top_left.y;

        // Create entity
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Value>(entity, { name, VarType::float64 }); //todo: make add int type
        scene.add_component<MouseInteract>(entity);
        scene.add_component<MultiHitbox>(entity, multi_hitbox);
        scene.add_component<Combobox>(entity, combobox);
        return entity;
    }

    inline EntityID create_box(
        Scene& scene,
        const std::string& name,
        const Transform& transform,
        const Box& box
    ) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, transform);
        scene.add_component<Box>(entity, box);
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
            renderer.draw_box_textured(*transform, sprite->sprites[0].tex_path, sprite->sprites[0].tex_type, transform->top_left, transform->bottom_right, color, transform->depth + 0.001f, transform->anchor);
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
                    if (text->text_length < 32) {
                        delete text->text;
                        text->text = new wchar_t[32];
                        text->text[0] = 'A';
                        text->text[1] = '\0';
                    }

                    //If all parts of the range are a whole number, print as if it were an integer
                    swprintf_s(text->text, 32, L"%.2f", val);
                    if (range) {
                        wchar_t filter[] = L"%.xf";
                        filter[2] = L'0' + range->visual_decimal_places;
                        swprintf_s(text->text, 32, filter, val);
                    }
                }
            }

            // Calculate position relative to top_left
            glm::vec2 transform_top_left = transform->top_left + text->margins;
            glm::vec2 transform_bottom_right = transform->bottom_right - text->margins;
            const glm::vec2 top_left = transform_top_left + glm::vec2(renderer.resolution()) * anchor_offsets[static_cast<size_t>(transform->anchor)];
            const glm::vec2 offset_from_top_left = (transform_bottom_right - transform_top_left) * anchor_offsets[static_cast<size_t>(text->ui_anchor)];
            renderer.draw_text({ {-9999, -9999}, {9999, 9999}, transform->depth, transform->anchor }, text->text, top_left + offset_from_top_left, text->scale, text->color, transform->depth, AnchorPoint::top_left, text->text_anchor);
#ifdef _DEBUG
            renderer.draw_circle_solid(*transform, top_left, { 4,4 }, { 1,0,1,1 });
#endif
        }
    }

    inline void system_comp_special_render(Scene& scene, Renderer& renderer, Input& input) {
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
            renderer.draw_circle_solid(*transform, center, scale, { 1, 1, 1, 1 }, transform->depth + 0.0002f, transform->anchor);
            renderer.draw_circle_line(*transform, center, scale, { 0, 0, 0, 1 }, 2, transform->depth + 0.0001f, transform->anchor);
            renderer.draw_line(*transform, center, line_b, { 0, 0, 0, 1 }, 4, transform->depth + 0.0001f);
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
            float margin = 8;
            if (draggable && draggable->is_horizontal)
            {
                const float dist_left = static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * (scale.x - margin);
                renderer.draw_line(*transform, center + glm::vec2{ scale.x, 0 }, center - glm::vec2{ scale.x, 0 }, { 0, 0, 0, 1 }, 4, transform->depth + 0.0002f);
                renderer.draw_line(*transform, center + glm::vec2{ scale.x, 0 }, center - glm::vec2{ scale.x, 0 }, { 1, 1, 1, 1 }, 2, transform->depth + 0.0001f);
                renderer.draw_box_solid(*transform, center + glm::vec2{ dist_left - 10, -20 }, center + glm::vec2{ dist_left + 10, +20 }, { 1,1,1,1 });
            }
            else {
                const float dist_bottom = static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * -(scale.y - margin);
                renderer.draw_line(*transform, center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y}, { 0, 0, 0, 1 }, 4, transform->depth + 0.0002f);
                renderer.draw_line(*transform, center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y}, { 1, 1, 1, 1 }, 2, transform->depth + 0.0001f);
                renderer.draw_box_solid(*transform, center + glm::vec2{ -20, dist_bottom - 10 }, center + glm::vec2{ +20, dist_bottom + 10 }, { 1,1,1,1 });
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
                const auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);
                //if (multi_hitbox != nullptr && i == radio_button->current_selected_index) {
                if (multi_hitbox != nullptr) {
                    if (multi_hitbox->click_states[i] == ClickState::hover) {
                        color *= 0.9f;
                    }
                    if (multi_hitbox->click_states[i] == ClickState::click) {
                        color *= 0.7f;
                    }
                };

                // Draw the circle outline for each of them
                renderer.draw_circle_line(*transform, circle_base_offset + glm::vec2(0, vertical_spacing * i), glm::vec2(outline_circle_radius), color, 2.0f, transform->depth, transform->anchor);

                // Draw the text
                renderer.draw_text(*transform, radio_button->options[i], circle_base_offset + glm::vec2(0, vertical_spacing * i) + glm::vec2(outline_circle_radius + text_margin, 0), { 2, 2 }, color, transform->depth, AnchorPoint::top_left, AnchorPoint::left);

                // Draw selected circle
                if (i == radio_button->current_selected_index) {
                    renderer.draw_circle_solid(*transform, circle_base_offset + glm::vec2(0, vertical_spacing * i), glm::vec2(selected_circle_radius), color, transform->depth, transform->anchor);
                }
            }
        }

        // Combobox
        for (const auto entity : scene.view<Transform, Combobox, MultiHitbox, Value>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* combobox = scene.get_component<Combobox>(entity);
            auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);
            auto* value = scene.get_component<Value>(entity);

            // Determine a nice color based on what the mouse is doing
            glm::vec4 top_color = { 1, 1, 1, 1 };

            // todo: use actual color schemes instead of hard coded magic numbers
            if (multi_hitbox->click_states[0] == ClickState::hover) {
                top_color *= 0.9f;
            }
            if (multi_hitbox->click_states[0] == ClickState::click) {
                top_color *= 0.7f;
            }

            // Render the button
            glm::vec2 box_top_left = transform->top_left;
            glm::vec2 box_bottom_right = { transform->bottom_right.x, transform->top_left.y + combobox->button_height };
            glm::vec2 arrow_center = { transform->bottom_right.x - 30.f, transform->top_left.y + (combobox->button_height / 2) + 8 };
            glm::vec2 text_offset = { 8, (combobox->button_height / 2) };
            glm::vec2 arrow_offset = { 16, -16 };
            renderer.draw_box_solid(*transform, box_top_left, box_bottom_right, top_color, transform->depth + 0.01f, transform->anchor);
            renderer.draw_box_line(*transform, box_top_left, box_bottom_right, { 0, 0, 0, 1 }, transform->depth, 0, transform->anchor);
            renderer.draw_text(*transform, combobox->list_items[combobox->current_selected_index], transform->top_left + text_offset, {2, 2}, {0, 0, 0, 0}, transform->depth - 0.01f, transform->anchor, AnchorPoint::left);
            renderer.draw_line(*transform, arrow_center, arrow_center + arrow_offset * glm::vec2(+1, 1), {0, 0, 0, 1}, 2, transform->depth - 0.01f, transform->anchor);
            renderer.draw_line(*transform, arrow_center, arrow_center + arrow_offset * glm::vec2(-1, 1), {0, 0, 0, 1}, 2, transform->depth - 0.01f, transform->anchor);

            // Debug
#ifdef _DEBUG
            for (size_t i = 0; i < multi_hitbox->n_hitboxes; ++i) {
                renderer.draw_box_line(*transform, transform->top_left + multi_hitbox->hitboxes[i].top_left, transform->top_left + multi_hitbox->hitboxes[i].bottom_right, {1, 1, 0, 1}, 2.0f);
            }
#endif

            // Render the list if necessary
            size_t start_index = size_t(combobox->current_scroll_position / combobox->item_height);
            size_t end_index = start_index + combobox->list_height / combobox->item_height + 1;
            if (abs(transform->bottom_right.y - transform->top_left.y - combobox->button_height) > 1.0f) {
                for (size_t i = start_index; i <= end_index; ++i)
                {
                    // Stop if end of list was reached
                    if (i >= combobox->list_items.size())
                        break;

                    // Get transform information
                    box_top_left = transform->top_left + glm::vec2(0, combobox->button_height + (combobox->item_height * i) - combobox->current_scroll_position);
                    box_bottom_right = { transform->bottom_right.x, box_top_left.y + combobox->item_height };
                    text_offset = { 8, combobox->item_height / 2.0f };

                    // Determine a nice color based on what the mouse is doing
                    glm::vec4 color = { 1, 1, 1, 1 };

                    // If this is the currently selected entry, darken it a bit
                    if (i == combobox->current_selected_index) {
                        color *= 0.8f;
                    }

                    // Create a hitbox for the current item
                    Hitbox curr_item_hitbox{};
                    curr_item_hitbox.top_left = renderer.apply_anchor_in_pixel_space(box_top_left, transform->anchor);
                    curr_item_hitbox.bottom_right = renderer.apply_anchor_in_pixel_space(box_bottom_right, transform->anchor);

                    // If the mouse is over it, change the color based on the mouse
                    if (curr_item_hitbox.intersects(input.mouse_pos(MouseRelative::window))) {
                        if (multi_hitbox->click_states[1] == ClickState::hover) {
                            color *= 0.9f;
                        }
                        if (multi_hitbox->click_states[1] == ClickState::click) {
                            // This is a bit cursed, but it'll have to do
                            // We will actually update the combobox selected index in the rendering code, since we already do a ton of logic here to figure out where the mouse is anyway
                            color *= 0.7f;
                            combobox->current_selected_index = i;
                            combobox->is_list_open = false;
                            value->get_as_ref<double>() = static_cast<double>(i);
                            break;
                        }
                    }

                    // Draw the boxes
                    renderer.draw_box_solid(*transform, box_top_left + glm::vec2(+1, 0), box_bottom_right + glm::vec2(-1, -1), color, transform->depth + 0.03f, transform->anchor);
                    renderer.draw_box_line(*transform, box_top_left, box_bottom_right, { 0, 0, 0, 1 }, transform->depth + 0.03f, 0, transform->anchor);
                    renderer.draw_text(*transform, combobox->list_items[i], box_top_left + text_offset, { 2, 2 }, { 0, 0, 0, 1 }, transform->depth + 0.02f, transform->anchor, AnchorPoint::left);
                }
            }
        }

        // Box
        for (const auto entity : scene.view<Transform, Box>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* box = scene.get_component<Box>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);

            // Hovering and clicking affects color
            float multiply = 1.0f;
            if (mouse_interact) {
                switch (mouse_interact->state) {
                case ClickState::hover:
                    multiply = 0.8f;
                    break;
                case ClickState::click:
                    multiply = 0.6f;
                    break;
                default:
                    break;
                }
            }

            renderer.draw_box_solid(*transform, transform->top_left, transform->bottom_right, box->color_inner * multiply, transform->depth + 0.001f, transform->anchor);
            renderer.draw_box_line(*transform, transform->top_left, transform->bottom_right, box->color_outer, box->thickness, transform->depth, transform->anchor);
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
            // If we're hovering over the element and we middle click, AND the component has a value, set that value to default
            auto* value = scene.get_component<Value>(entity);
            auto* range = scene.get_component<NumberRange>(entity);
            if (input.mouse_down(2) && value && range && mouse_interact->state == ClickState::hover) {
                if (value->type == VarType::float64) {
                    value->set<double>(range->default_value);
                }
            }
        }

        // Handle multi-hitbox components
        for (const auto entity : scene.view<Transform, MultiHitbox>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);

            // Check for each hitbox
            for (size_t i = 0; i < multi_hitbox->n_hitboxes; ++i) {
                // Transform the hitbox from local space to window space
                Hitbox hitbox = multi_hitbox->hitboxes[i];
                hitbox.top_left += renderer.apply_anchor_in_pixel_space(transform->top_left, transform->anchor);
                hitbox.bottom_right += renderer.apply_anchor_in_pixel_space(transform->top_left, transform->anchor);

                // See if it intersects
                if (hitbox.intersects(input.mouse_pos(MouseRelative::window)))
                {
                    // If the user is clicking
                    if (input.mouse_held(0)) {
                        multi_hitbox->click_states[i] = ClickState::click;
                    }

                    // Otherwise hover
                    else {
                        multi_hitbox->click_states[i] = ClickState::hover;
                    }
                }
                else {
                    multi_hitbox->click_states[i] = ClickState::idle;
                }
            }
        }
    }

    inline void system_comp_draggable_clickable(Scene& scene, Input& input) {
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

        // Handle scrollable components like sliders, numberboxes
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
        // Handle radio buttons
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

    inline void system_comp_combobox(Scene& scene, Input& input, float delta_time, bool& combobox_handled) {
        // Handle combobox
        for (const auto entity : scene.view<Transform, Value, Combobox, MultiHitbox>()) {
            auto* transform = scene.get_component<Transform>(entity);
            auto* combobox = scene.get_component<Combobox>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);
            auto* value = scene.get_component<Value>(entity);

            // Make sure it also has a mouse interact component
            if (mouse_interact == nullptr) {
                continue;
            }

            // If the mouse is clicked on in general
            if (input.mouse_down(0)) {
                // If it's the top part of the combobox, toggle it
                if (multi_hitbox->click_states[0] == ClickState::click) {
                    combobox->is_list_open = !combobox->is_list_open;
                    combobox_handled = true;
                }

                // If it's outside the combobox in general, close it
                else if (combobox->is_list_open){
                    combobox->is_list_open = false;
                    combobox_handled = true;
                }
            }

            // Make sure we have full focus when the mouse is on it
            if (mouse_interact->state != ClickState::idle) {
                combobox_handled = true;
            }

            // Handle scrolling
            if (input.mouse_wheel() != 0) {
                // If we are hovered over the list, scroll the list
                if (multi_hitbox->click_states[1] == ClickState::hover) {
                    combobox->target_scroll_position -= input.mouse_wheel() * combobox->item_height * 1.25f;
                    float min = 0.0f;
                    float max = combobox->item_height * combobox->list_items.size() - combobox->list_height;
                    max = std::max(min, max);
                    combobox->target_scroll_position = std::clamp(combobox->target_scroll_position, min, max);
                }

                // Otherwise if we are hovered over the button, change the index
                if (multi_hitbox->click_states[0] == ClickState::hover) {
                    combobox->current_selected_index -= (int)input.mouse_wheel();
                    combobox->target_scroll_position = (combobox->current_selected_index - 0.5f) * combobox->item_height ;
                    float min = 0.0f;
                    float max = combobox->item_height * combobox->list_items.size() - combobox->list_height;
                    max = std::max(min, max);
                    combobox->target_scroll_position = std::clamp(combobox->target_scroll_position, min, max);
                    combobox->current_selected_index = std::clamp(combobox->current_selected_index, 0, int(combobox->list_items.size()) - 1);
                    value->get_as_ref<double>() = combobox->current_selected_index;
                }
            }

            // Handle transform
            float target_bottom = transform->top_left.y + combobox->button_height + (combobox->is_list_open * combobox->list_height);
            transform->bottom_right.y = std::lerp(transform->bottom_right.y, target_bottom, 1.0f - pow(2.0f, -delta_time * 75.0f));

            // Interpolate towards the scroll
            combobox->current_scroll_position = std::lerp(combobox->current_scroll_position, combobox->target_scroll_position, 1.0f - pow(2.0f, -delta_time * 25.0f));
        }
    }

    inline void update_entities(Scene& scene, Renderer& renderer, Input& input, float delta_time) {
        // Render sprites
        system_comp_sprite(scene, renderer);
        system_comp_special_render(scene, renderer, input);

        // Render text
        system_comp_text(scene, renderer);

        // Handle clickable components
        system_comp_mouse_interact(scene, renderer, input);

        // Handle comboboxes - special case: if a combobox is interacted with, don't handle any other ones
        bool combobox_handled = false;
        system_comp_combobox(scene, input, delta_time, combobox_handled);

        if (combobox_handled == false) {
            // Handle other mouse interactable components
            system_comp_draggable_clickable(scene, input);

            // Handle radio buttons
            system_comp_radio_buttons(scene, renderer, input);
        }

        // Debug
#ifdef _DEBUG
        for (const auto entity : scene.view<Transform>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            renderer.draw_box_line(*transform, transform->top_left, transform->bottom_right, { 1, 0, 1, 1 }, 1, 0, transform->anchor);
        }
#endif
    }
}
