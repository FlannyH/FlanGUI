#pragma once
#include <vector>

#include "GL/glcorearb.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Flan {
    struct Vertex {
        glm::vec3 pos{};
        glm::vec2 tc{};
        glm::vec4 color{};
        glm::vec4 clip_rect = { -99999, -99999, 99999, 99999 };
    };

    struct Triangle {
        Vertex a;
        Vertex b;
        Vertex c;
    };

    struct Font {
        GLuint texture_id;
        uint16_t grid_w;
        uint16_t grid_h;
        std::vector<int> widths;
    };

    enum class ShaderType {
        vertex,
        pixel,
        geometry,
        compute
    };

    enum class TextureType {
        stretch,
        tile,
        slice
    };

    struct Texture {
        GLuint id;
        glm::ivec2 res;
    };

    struct ClipRect {
        glm::vec2 top_left;
        glm::vec2 bottom_right;
    };
}
