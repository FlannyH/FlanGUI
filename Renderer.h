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

    class Renderer
    {
    public:
        void init();
        void init(GLFWwindow* window); // Init the renderer using an existing window
        void begin_frame();
        void gl_error();
        void end_frame();
        void flip_buffers();
        void draw_line(glm::vec2 a, glm::vec2 b, glm::vec4 color, float width = 1.0f, float depth = 0.0f/*, AnchorPoint anchor*/); //TODO: anchorpoint
        void draw_linebox(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float width = 1.0f, float depth = 0.0f/*, AnchorPoint anchor*/); //TODO: anchorpoint
        void draw_linecircle(glm::vec2 center, glm::vec2 scale, glm::vec4 color, float width = 1.0f, float depth = 0.0f);
        void draw_solidbox(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float width = 1.0f, float depth = 0.0f/*, AnchorPoint anchor*/); //TODO: anchorpoint
        void draw_polygon(std::vector<Vertex> verts);
        void init_text_lut();
        void draw_text(const std::wstring& text, glm::vec2 pos, glm::vec2 scale, glm::vec4 color, float depth/*, AnchorPoint anchor*/); //TODO: anchorpoint
        void* alloc_temp(size_t size, size_t align = 16);
        bool load_font(std::string path);
        GLuint shader_from_file(std::string path);
        GLuint shader_from_string(std::string vert, std::string frag);
        bool shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program);
        bool shader_part_from_string(const std::string& string, ShaderType type, const GLuint& program);
        std::vector<Triangle> _line_queue;
        GLFWwindow* _window = nullptr;
        const glm::ivec2 _res = { 1280, 720 };
        GLuint shader;
        GLuint vao;
        GLuint vbo;
        Font font;
        std::map<wchar_t, std::vector<int>> wchar_lut;
    };
}