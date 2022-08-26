#pragma once
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
        void draw_text(const std::string& text, glm::vec2 pos_pixels, float width = 1.0f, float depth = 1.0f/*, AnchorPoint anchor*/); //TODO: anchorpoint
        void* alloc_temp(size_t size, size_t align = 16);
        GLuint load_shader(std::string path);
        bool load_shader_part(const std::string& path, ShaderType type, const GLuint& program);
        std::vector<Triangle> _line_queue;
        GLFWwindow* _window = nullptr;
        const glm::ivec2 _res = { 1280, 720 };
        GLuint shader;
        GLuint vao;
        GLuint vbo;
    };
}