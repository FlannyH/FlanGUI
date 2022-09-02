#pragma once
#include <functional>
#include <utility>

#include "ComponentSystem.h"
#include "Input.h"
#include "Renderer.h"
#include "glm/vec2.hpp"

namespace Flan {
    struct Transform {
        Transform(const glm::vec2 tl, const glm::vec2 br, const float dpth = 0.0f, const AnchorPoint anch = AnchorPoint::top_left) {
            top_left = tl;
            bottom_right = br;
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
        ClickState state = ClickState::idle;
        std::function<void()> on_click;
    };

    struct SpriteRender {
        SpriteRender(const std::string& path, const int n_tex) {
            tex_path = path;
            n_textures = n_tex;
        }
        std::string tex_path;
        int n_textures = 0; // Number of animation frames, sliced horizontally
        TextureType tex_type = TextureType::stretch;
    };

    struct Text {
        Text(const std::wstring& txt, const glm::vec2 scl = { 2, 2 }, const glm::vec4 col = { 1, 1, 1, 1 }, const AnchorPoint txt_anchr = AnchorPoint::center, const AnchorPoint ui_anchr = AnchorPoint::center) {
            text = txt;
            text_anchor = txt_anchr;
            ui_anchor = ui_anchr;
            color = col;
            scale = scl;
        }
        std::wstring text;
        AnchorPoint text_anchor;
        AnchorPoint ui_anchor;
        glm::vec4 color{};
        glm::vec2 scale{};
    };

    inline EntityID button(Scene& scene, const glm::vec2 top_left, const glm::vec2 bottom_right, std::function<void()> func, const float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left, const std::wstring& text = L"", glm::vec4 text_color = {1, 1, 1, 1}, glm::vec2 text_scale = {2, 2}) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, { top_left, bottom_right, depth, anchor });
        scene.add_component<Clickable>(entity, {std::move(func)});
        scene.add_component<SpriteRender>(entity, { "button.png", 1 });
        scene.add_component<Text>(entity, { text, text_scale, text_color });
        return entity;
    }

    inline void system_comp_sprite(Scene& scene, Renderer& renderer) {
        for (const auto entity : scene.view<Transform, SpriteRender>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* sprite = scene.get_component<SpriteRender>(entity);
            // If it has a clickable component, use that to render the button
            glm::vec4 color = { 1, 1, 1, 1 };
            if (const auto* clickable = scene.get_component<Clickable>(entity)) {
                if (clickable->state == ClickState::hover) {
                    color *= 0.9f;
                }
                if (clickable->state == ClickState::click) {
                    color *= 0.7f;
                }
            }
            renderer.draw_box_textured(sprite->tex_path, sprite->tex_type, transform->top_left, transform->bottom_right, color, transform->depth + 0.001f, transform->anchor);
        }
    }

    inline void system_comp_text(Scene& scene, Renderer& renderer) {
        for (const auto entity : scene.view<Transform, Text>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* text = scene.get_component<Text>(entity);
            const glm::vec2 anchor_offsets[] = {
                {0.5f, 0.5f}, // center
                {0.0f, 1.0f}, // top left
                {0.5f, 1.0f}, // top
                {1.0f, 1.0f}, // top right
                {1.0f, 0.5f}, // right
                {1.0f, 0.0f}, // bottom right
                {0.5f, 0.0f}, // bottom
                {0.0f, 0.0f}, // bottom left
                {0.0f, 0.5f}, // left
            };
            const glm::vec2 offset_from_top_left = transform->top_left + (transform->bottom_right - transform->top_left) * anchor_offsets[static_cast<size_t>(transform->anchor)];
            renderer.draw_text(text->text, offset_from_top_left, text->scale, text->color, transform->depth, text->ui_anchor, text->text_anchor);
        }
    }

    inline void system_comp_clickable(Scene& scene, Renderer& renderer, const Input& input) {
        for (const auto entity : scene.view<Transform, Clickable>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            auto* clickable = scene.get_component<Clickable>(entity);

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
            const glm::vec2 mouse_pos = input.mouse_pos(MouseRelative::window);
            const glm::vec2 anch_off = static_cast<glm::vec2>(renderer.resolution()) * (anchor_offsets[static_cast<size_t>(transform->anchor)] + glm::vec2{1, 1}) / 2.f;
            glm::vec2 tl_ = anch_off + transform->top_left;
            glm::vec2 br_ = anch_off + transform->bottom_right;
            const glm::vec2 tl = { std::min(tl_.x, br_.x), std::min(tl_.y, br_.y) };
            const glm::vec2 br = { std::max(tl_.x, br_.x), std::max(tl_.y, br_.y) };
            renderer.draw_circle_solid(mouse_pos, { 5, 5 }, { 1, 0, 1, 1 });
            renderer.draw_circle_line(mouse_pos, { 5, 5 }, { 0, 1, 0, 1 });
            renderer.draw_line(mouse_pos - glm::vec2{0, 32}, mouse_pos + glm::vec2{0, 32}, { 0, 1, 1, 1 });
            renderer.draw_line(mouse_pos - glm::vec2{32, 0}, mouse_pos + glm::vec2{32, 0}, { 0, 1, 1, 1 });

            if (clickable->state != ClickState::click) {
                if (mouse_pos.x >= tl.x &&
                    mouse_pos.y >= tl.y &&
                    mouse_pos.x <= br.x &&
                    mouse_pos.y <= br.y) {
                    clickable->state = ClickState::hover;
                } else {
                    clickable->state = ClickState::idle;
                }
            }
            if (input.mouse_down(0)) {
                if (mouse_pos.x >= tl.x &&
                    mouse_pos.y >= tl.y &&
                    mouse_pos.x <= br.x &&
                    mouse_pos.y <= br.y) {
                    clickable->state = ClickState::click;
                }
            }
            if (input.mouse_up(0) && clickable->state == ClickState::click) {
                if (mouse_pos.x >= tl.x &&
                    mouse_pos.y >= tl.y &&
                    mouse_pos.x <= br.x &&
                    mouse_pos.y <= br.y) {
                    clickable->on_click();
                }
                clickable->state = ClickState::idle;
            }
        }
    }

    inline void update_entities(Scene& scene, Renderer& renderer, const Input& input) {
        // Render sprites
        system_comp_sprite(scene, renderer);

        // Render text
        system_comp_text(scene, renderer);

        // Handle clickable components
        system_comp_clickable(scene, renderer, input);
    }
}
