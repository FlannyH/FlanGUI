#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "GL/glcorearb.h"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <glfw/glfw3native.h>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "CommonStructs.h"
#include "RendererStructs.h"

namespace Flan {
    class Renderer {
    public:
        //---General---
        void init(bool invisible = false);
        void init(int w, int h, bool invisible = false, HMODULE dll_handle = nullptr);
        void init(GLFWwindow* window); // Init the renderer using an existing window
        void begin_frame();
        static void gl_error();
        void end_frame() const;
        void init_luts();
        void flip_buffers() const;
        [[nodiscard]] GLFWwindow* window() const { return m_window; }
        [[nodiscard]] glm::ivec2 resolution() const { return m_res; }

        //---Resource Management---
        static GLuint shader_from_file(const std::string& path);
        [[nodiscard]] GLuint shader_from_resource(const std::string& path) const;
        //static GLuint shader_from_string(const std::string& vert, const std::string& frag);
        static bool shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program);
        [[nodiscard]] bool shader_part_from_resource(const std::string& name, ShaderType type, const GLuint& program) const;
        static bool shader_part_from_string(const std::string& string, ShaderType type, const GLuint& program);
        bool load_texture(const std::string& path, Texture& handle) const;
        bool load_font(const std::string& path);

        //---Drawing Functions---
        void draw_line(Transform transform, glm::vec2 a, glm::vec2 b, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_line(Transform transform, glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_solid(Transform transform, glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_box_textured(Transform transform, const::std::string& texture, TextureType tex_type, glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, float depth = 0.f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_polygon_textured(Transform transform, Vertex* verts, size_t n_verts, const std::string& texture, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_circle_line(Transform transform, glm::vec2 center, glm::vec2 scale, glm::vec4 color, float width = 1.0f, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_circle_solid(Transform transform, glm::vec2 center, glm::vec2 scale, glm::vec4 color, float depth = 0.0f, AnchorPoint anchor = AnchorPoint::top_left);
        void draw_flat_polygon(Transform transform, Vertex* verts, size_t n_verts, AnchorPoint anchor);
        void get_texture(const std::string& texture);
        void draw_text(Transform transform, const std::wstring& text, glm::vec2 pos, glm::vec2 scale, glm::vec4 color, float depth, AnchorPoint ui_anchor = AnchorPoint::top_left, AnchorPoint text_anchor = AnchorPoint::top_left);
        void set_clipping_rectangle(bool enabled, glm::vec2 top_left = {0.0f, 0.0f}, glm::vec2 bottom_right = {0.0f, 0.0f});
        [[nodiscard]] glm::vec2 apply_anchor(glm::vec2 pos, AnchorPoint anchor) const;
        [[nodiscard]] glm::vec2 pixels_to_normalized(glm::vec2 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
        [[nodiscard]] glm::vec3 pixels_to_normalized(glm::vec3 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
        [[nodiscard]] glm::vec2 apply_anchor_in_pixel_space(glm::vec2 pos, AnchorPoint anchor = AnchorPoint::top_left) const;
        [[nodiscard]] float get_font_height() const { return m_font.grid_h; }
    private:

        std::vector<Triangle> m_flat_queue;
        std::map<GLuint, std::vector<Triangle>> m_textured_queue;
        GLFWwindow* m_window = nullptr;
        const glm::ivec2 m_res_ref = { 1280, 720 };
        glm::ivec2 m_res = m_res_ref;
        GLuint m_shader{};
        GLuint m_vao{};
        GLuint m_vbo{};
        Font m_font{};
        ClipRect m_clipping_rectangle{};
        #define SINE_LUT_RESOLUTION 32
        std::array<float, SINE_LUT_RESOLUTION> m_sine_lut{};
        std::map<wchar_t, std::vector<int>> m_wchar_lut;
        std::map<std::string, Texture> m_textures;
        HMODULE m_dll{};
    };
}
