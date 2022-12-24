#pragma once
#include <glm/glm.hpp>
namespace Flan {
    enum class AnchorPoint {
        center = 0,
        top_left,
        top,
        top_right,
        right,
        bottom_right,
        bottom,
        bottom_left,
        left,
    };

    struct Transform {
        Transform(const glm::vec2 tl, const glm::vec2 br, const float dpth = 0.5f, const AnchorPoint anch = AnchorPoint::top_left) {
            const float x_min = std::min(tl.x, br.x);
            const float x_max = std::max(tl.x, br.x);
            const float y_min = std::min(tl.y, br.y);
            const float y_max = std::max(tl.y, br.y);
            top_left = { x_min, y_min };
            bottom_right = { x_max, y_max };
            anchor = anch;
            depth = dpth;
        }
        glm::vec2 top_left{}, bottom_right{};
        float depth{};
        AnchorPoint anchor{};
    };
}