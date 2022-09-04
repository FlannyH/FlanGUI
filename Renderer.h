#pragma once
#include <array>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "GL/glcorearb.h"
#include "glfw/glfw3.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Flan {
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 tc;
        glm::vec4 color;
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

    enum class TextureType {
        stretch,
        tile,
        slice
    };

    struct Texture {
        GLuint id;
        glm::ivec2 res;
    };

    class Renderer {
    public:
        //---General---
        void init();
        void init(GLFWwindow* window); // Init the renderer using an existing window
        void begin_frame();
        static void gl_error();
        void end_frame() const;
        void init_luts();
        void flip_buffers() const;
        [[nodiscard]] GLFWwindow* window() const { return _window; }
        [[nodiscard]] glm::ivec2 resolution() const { return _res; }

        //---Resource Management---
        static GLuint shader_from_file(const std::string& path);
        static GLuint shader_from_string(const std::string& vert, const std::string& frag);
        static bool shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program);
        static bool shader_part_from_string(const std::string& string, ShaderType type, const GLuint& program);
        static bool load_texture(const std::string& path, Texture& handle);
        bool load_font(const std::string& path);

        //---Drawing Functions---
        void draw_line(glm::vec2 a, glm::vec2 b, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_line(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_solid(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_textured(const::std::string& texture, TextureType tex_type, glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float depth = 0.f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_polygon_textured(std::vector<Vertex> verts, const std::string& texture, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_circle_line(glm::vec2 center, glm::vec2 scale, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_circle_solid(glm::vec2 center, glm::vec2 scale, glm::vec4 color, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_flat_polygon(std::vector<Vertex> verts, AnchorPoint anchor = AnchorPoint::top_left);
        void get_texture(const std::string& texture);
        void draw_text(const std::wstring& text, glm::vec2 pos, glm::vec2 scale, glm::vec4 color, float depth, AnchorPoint ui_anchor = AnchorPoint::top_left, AnchorPoint text_anchor = AnchorPoint::top_left);
        [[nodiscard]] glm::vec2 apply_anchor(glm::vec2 pos, AnchorPoint anchor);
        [[nodiscard]] glm::vec2 pixels_to_normalized(glm::vec2 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
        [[nodiscard]] glm::vec3 pixels_to_normalized(glm::vec3 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
    private:

        std::vector<Triangle> _flat_queue;
        std::map<GLuint, std::vector<Triangle>> _textured_queue;
        GLFWwindow* _window = nullptr;
        const glm::ivec2 _res_ref = { 1280, 720 };
        glm::ivec2 _res = _res_ref;
        GLuint _shader{};
        GLuint _vao{};
        GLuint _vbo{};
        Font _font{};
        #define SINE_LUT_RESOLUTION 32
        std::array<float, SINE_LUT_RESOLUTION> _sine_lut{};
        std::map<wchar_t, std::vector<int>> _wchar_lut;
        std::map<std::string, Texture> _textures;
    };
}
