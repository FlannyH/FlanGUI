#pragma once
#include <functional>

#include "ComponentSystem.h"
#include "Input.h"
#include "Renderer.h"
#include "glm/vec2.hpp"

namespace Flan {
    struct Transform {
        Transform(const glm::vec2 tl, const glm::vec2 br, const float depth = 0.0f, const AnchorPoint anch = AnchorPoint::top_left) {
            top_left = tl;
            bottom_right = br;
            anchor = anch;
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
        Clickable(const std::function<void()>& click) {
            on_click = click;
        }
        ClickState state = ClickState::idle;
        std::function<void()> on_click;
    };

    struct SpriteRender {
        SpriteRender(const std::string& path, const int n_tex) {
            tex_path = path;
            n_textures = n_tex;
        }
        std::string tex_path = "";
        int n_textures = 0; // Number of animation frames, sliced horizontally
    };

    inline EntityID button(Scene& scene, const glm::vec2 top_left, const glm::vec2 bottom_right, const std::function<void()>& func, const float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left) {
        const EntityID entity = scene.new_entity();
        scene.add_component<Transform>(entity, { top_left, bottom_right, depth, anchor });
        scene.add_component<Clickable>(entity, { func });
        scene.add_component<SpriteRender>(entity, { "test.png", 1 });
        return entity;
    }

    inline void update_entities(Scene& scene, Renderer& renderer, Input& input) {
        // Render sprites
        for (const auto entity : scene.view<Transform, SpriteRender>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* sprite = scene.get_component<SpriteRender>(entity);
            renderer.draw_box_textured(sprite->tex_path, transform->top_left, transform->bottom_right, { 1, 1, 1, 0 }, 0.0f, transform->depth, transform->anchor);
        }

        // Handle clickable components
        for (const auto entity : scene.view<Transform, Clickable>()) {
            const auto* transform = scene.get_component<Transform>(entity);
            const auto* clickable = scene.get_component<Clickable>(entity);

        }
    }
}
