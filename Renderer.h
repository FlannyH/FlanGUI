#pragma once
#include <map>
#include <string>
#include <vector>

#include "GL/glcorearb.h"
#include "GLFW/glfw3.h"
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

    class Renderer {
    public:
        //---General---
        void init();
        void init(GLFWwindow* window); // Init the renderer using an existing window
        void begin_frame();
        static void gl_error();
        void end_frame() const;
        void init_text_lut();
        void flip_buffers() const;
        GLFWwindow* window() const { return _window; }

        //---Resource Management---
        static GLuint shader_from_file(const std::string& path);
        static GLuint shader_from_string(const std::string& vert, const std::string& frag);
        static bool shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program);
        static bool shader_part_from_string(const std::string& string, ShaderType type, const GLuint& program);
        static bool load_texture(const std::string& path, GLuint& handle);
        bool load_font(const std::string& path);

        //---Drawing Functions---
        void draw_line(glm::vec2 a, glm::vec2 b, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_linebox(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_linecircle(glm::vec2 center, glm::vec2 scale, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_solidbox(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float outline_width = 0.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_flat_polygon(std::vector<Vertex> verts, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_texturebox(const std::string& texture, glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float outline_width = 0.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_textured_polygon(std::vector<Vertex> verts, const std::string& texture, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_text(const std::wstring& text, glm::vec2 pos, glm::vec2 scale, glm::vec4 color, float depth, AnchorPoint anchor = AnchorPoint::top_left);
    private:
        glm::vec2 pixels_to_normalized(glm::vec2 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
        glm::vec3 pixels_to_normalized(glm::vec3 pos, AnchorPoint anchor = AnchorPoint::top_left) const;

        std::vector<Triangle> _flat_queue;
        std::map<GLuint, std::vector<Triangle>> _textured_queue;
        GLFWwindow* _window = nullptr;
        const glm::ivec2 _res_ref = { 1280, 720 };
        glm::ivec2 _res = _res_ref;
        GLuint _shader{};
        GLuint _vao{};
        GLuint _vbo{};
        Font _font{};
        std::map<wchar_t, std::vector<int>> _wchar_lut;
        std::map<std::string, GLuint> _textures;
    };
}
