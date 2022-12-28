// ReSharper disable CppClangTidyPerformanceNoIntToPtr
#include "Renderer.h"

#include <fstream>
#include <string_view>
#include <utility>
#include <GL/gl3w.h>

#include "glm/geometric.hpp"
#include "glm/gtx/exterior_product.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ComponentsGUI.h"
#include "stb/stb_image.h"
#include "resource.h"


static void debug_callback_func(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam)
{
	// Skip some less useful info
	if (id == 131218)	// http://stackoverflow.com/questions/12004396/opengl-debug-context-performance-warning
		return;

	(void)(length);
	(void)(userParam);
	std::string source_string;
	std::string type_string;
	std::string severity_string;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_SOURCE_API: {
		source_string = "API";
		break;
	}
	case GL_DEBUG_SOURCE_APPLICATION: {
		source_string = "Application";
		break;
	}
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		source_string = "Window System";
		break;
	}
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		source_string = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		source_string = "Third Party";
		break;
	}
	case GL_DEBUG_SOURCE_OTHER: {
		source_string = "Other";
		break;
	}
	default: {
		source_string = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		type_string = "Error";
		break;
	}
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		type_string = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		type_string = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		type_string = "Portability";
		break;
	}
	case GL_DEBUG_TYPE_PERFORMANCE: {
		type_string = "Performance";
		break;
	}
	case GL_DEBUG_TYPE_OTHER: {
		type_string = "Other";
		break;
	}
	default: {
		type_string = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severity_string = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severity_string = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severity_string = "Low";
		break;
	}
	default: {
		severity_string = "Unknown";
		return;
	}
	}

	printf("[ERROR]\
GL Debug Callback:\n\
source:		%i:%s \n\
type:		%i:%s \n\
id:			%i \n\
severity:	%i:%s \n\
message:	%s\n", 
		source,	source_string.c_str(), 
		type, type_string.c_str(), 
		id, 
		severity, severity_string.c_str(), 
		message);
}


namespace Flan {
    bool read_file(const std::string& path, int& size, char*& data) {
        // Open file
        std::ifstream file;
        file.open(path.c_str(), std::ios::binary | std::ios::in);
        if (file.good() == false) return false;

        // Get size
        const std::streampos startpos = file.tellg();
        file.seekg(0, std::ios::end);
        size = static_cast<int>(file.tellg() - startpos);
        file.seekg(0, std::ios::beg);

        // Get data
        data = static_cast<char*>(malloc(size));
        if (!data) return false;
        file.read(data, size);

        return true;
    }

    bool read_resource(std::string name, int& size, char*& data, const std::wstring& type, HMODULE dll) {
        // Convert to wstring
        std::wstring wname;
        wname.resize(name.size() + 1);
        size_t converted_chars = 0;
        mbstowcs_s(&converted_chars, &wname[0], wname.size(), name.c_str(), name.size());

        // Find the resource
        HRSRC resource_handle = FindResource(dll, wname.c_str(), type.c_str());
        if (!resource_handle) return false;

        // Load it into memory
        HGLOBAL resource_data_handle = LoadResource(dll, resource_handle);
        if (!resource_data_handle) return false;

        // Get the pointer to the data
        LPVOID resource_data = LockResource(resource_data_handle);
        if (!resource_data) return false;

        // Get the size of the data
        DWORD resource_data_size = SizeofResource(dll, resource_handle);
        if (!resource_data_size) return false;

        // Return the data into the parameters
        size = resource_data_size;
        data = (char*)resource_data;

        return true;
    }
    const std::string vert_shader =
        "#version 330 core\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec3 i_position;\n"
        "layout (location = 1) in vec2 i_texcoord;\n"
        "layout (location = 2) in vec4 i_colour;\n"
        "out vec2 texcoord;\n"
        "out vec4 colour;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	gl_Position = vec4(i_position, 1);\n"
        "	texcoord = i_texcoord;\n"
        "	colour = i_colour;\n"
        "}";
    const std::string frag_shader =
        "#version 330 core\n"
        "precision mediump float;\n"
        "\n"
        "out vec4 frag_color;\n"
        "in vec2 texcoord;\n"
        "in vec4 colour;\n"
        "\n"
        "uniform sampler2D tex;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    frag_color = colour;\n"
        "}";

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

    void Renderer::init(bool invisible) {
        // Init + window settings
        glfwInit();
        if (invisible)
        {
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Debug
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

        GLFWwindow* window = glfwCreateWindow(_res.x, _res.y, "FlanGUI", nullptr, nullptr);
        glfwMakeContextCurrent(window);
        if (gl3wInit() != GL3W_OK) {
            printf("OpenGL error.\n");
            std::exit(1);
        }
        glDebugMessageCallback(debug_callback_func, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        init(window);
        glfwMakeContextCurrent(NULL);
    }

    void Renderer::init(int w, int h, bool invisible, HMODULE dll_handle)
    {
        _res.x = w;
        _res.y = h;
        dll = dll_handle;
        init(invisible);
    }

    void Renderer::init(GLFWwindow* window) {
        // Init basic rendering
        _window = window;
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glfwSwapInterval(0);
        //_shader = shader_from_file("Shaders\\sprite");
        _shader = shader_from_resource("sprite");
        load_font("font.png");
        //shader = shader_from_string(vert_shader, frag_shader);
        glUseProgram(_shader);

        // Create vertex buffer
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);

        // Setup vertex array
        glVertexAttribPointer(0, sizeof(Vertex::pos      ) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos      .x)));
        glVertexAttribPointer(1, sizeof(Vertex::tc       ) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tc       .x)));
        glVertexAttribPointer(2, sizeof(Vertex::color    ) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color    .x)));
        glVertexAttribPointer(3, sizeof(Vertex::clip_rect) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, clip_rect.x)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Renderer::begin_frame() {
        // Setup render context
        glfwMakeContextCurrent(_window);
        glfwPollEvents();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        // Handle window size changes
        glfwGetWindowSize(_window, &_res.x, &_res.y);
        glViewport(0, 0, _res.x, _res.y);

        // Clear queues
        _flat_queue.clear();
        _textured_queue.clear();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::gl_error() {
        const auto error = glGetError();
        if (error) {
            printf("ERROR: %i\n", error);
        }
    }

    void Renderer::end_frame() const {
        // Set resolution uniform
        auto location = glGetUniformLocation(_shader, "resolution");
        glUniform2iv(location, 1, &_res.x);

        // Render flat triangle queue
        glUseProgram(_shader);
        glBindTexture(GL_TEXTURE_2D, _font.texture_id);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Triangle) * _flat_queue.size()), _flat_queue.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_flat_queue.size() * 3));

        // Render textured triangle queue
        for (auto& entry : _textured_queue) {
            glUseProgram(_shader);
            glBindTexture(GL_TEXTURE_2D, entry.first);
            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Triangle) * entry.second.size()), entry.second.data(), GL_STATIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(entry.second.size()) * 3);
        }
        flip_buffers();
    }

    void Renderer::flip_buffers() const {
        glfwSwapBuffers(_window);
    }

    void Renderer::draw_line(Transform transform, glm::vec2 a, glm::vec2 b, glm::vec4 color, float width, float depth, AnchorPoint anchor) {
        Vertex v1{}, v2{}, v3{}, v4{};

        // Calculate normal
        glm::vec2 normal = (b - a);
        normal = { -normal.y, normal.x };
        normal = glm::normalize(normal) * width;

        // Create triangles
        v1.pos = glm::vec3(pixels_to_normalized(a - normal, anchor), depth);
        v2.pos = glm::vec3(pixels_to_normalized(b - normal, anchor), depth);
        v3.pos = glm::vec3(pixels_to_normalized(b + normal, anchor), depth);
        v4.pos = glm::vec3(pixels_to_normalized(a + normal, anchor), depth);
        v1.tc = { 0, 0 };
        v2.tc = { 0, 0 };
        v3.tc = { 0, 0 };
        v4.tc = { 0, 0 };
        v1.color = color;
        v2.color = color;
        v3.color = color;
        v4.color = color;
        v1.clip_rect = { transform.top_left.x, transform.top_left.y, transform.bottom_right.x, transform.bottom_right.y };
        v2.clip_rect = { transform.top_left.x, transform.top_left.y, transform.bottom_right.x, transform.bottom_right.y };
        v3.clip_rect = { transform.top_left.x, transform.top_left.y, transform.bottom_right.x, transform.bottom_right.y };
        v4.clip_rect = { transform.top_left.x, transform.top_left.y, transform.bottom_right.x, transform.bottom_right.y };
        _flat_queue.push_back({v1, v3, v2});
        _flat_queue.push_back({v1, v4, v3});
    }

    void Renderer::draw_box_line(Transform transform, const glm::vec2 top_left, const glm::vec2 bottom_right, const glm::vec4 color, const float width, const float depth, AnchorPoint anchor) {
        // Derive corners of the box
        const glm::vec2 tl = top_left;
        const glm::vec2 br = bottom_right;
        const glm::vec2 bl = { tl.x, br.y };
        const glm::vec2 tr = { br.x, tl.y };

        // Draw the lines
        const glm::vec2 x = { width, 0 };
        const glm::vec2 y = { x.y, x.x };
        draw_line(transform, tl - x, tr + x, color, width, depth, anchor);
        draw_line(transform, tr + y, br - y, color, width, depth, anchor);
        draw_line(transform, br + x, bl - x, color, width, depth, anchor);
        draw_line(transform, bl - y, tl + y, color, width, depth, anchor);
    }

    void Renderer::draw_circle_line(Transform transform, const glm::vec2 center, const glm::vec2 scale, const glm::vec4 color, const float width, const float depth, AnchorPoint anchor) {
        glm::vec2 points[SINE_LUT_RESOLUTION];

        // Generate points
        for (size_t i = 0; i < SINE_LUT_RESOLUTION; i++) {
            const size_t sx = i;
            const size_t cx = (sx + (SINE_LUT_RESOLUTION / 4)) % SINE_LUT_RESOLUTION;
            points[i] = center + (scale * glm::vec2{ _sine_lut[cx], _sine_lut[sx]});
        }

        // Draw lines
        for (int i = 0; i < SINE_LUT_RESOLUTION; i++) {
            draw_line(transform, points[i], points[(i + 1) % SINE_LUT_RESOLUTION], color, width, depth, anchor);
        }
    }

    void Renderer::draw_circle_solid(Transform transform, const glm::vec2 center, const glm::vec2 scale, const glm::vec4 color, const float depth, const AnchorPoint anchor) {
        static glm::vec2 points[SINE_LUT_RESOLUTION]; // todo: see if this can be static, or even just a C-style array, since it's a temporary array anyway

        // Generate points
        for (size_t i = 0; i < SINE_LUT_RESOLUTION; i++) {
            const size_t sx = i;
            const size_t cx = (sx + (SINE_LUT_RESOLUTION / 4)) % SINE_LUT_RESOLUTION;
            points[i] = center + (scale * glm::vec2{ _sine_lut[cx], _sine_lut[sx] });
        }

        // Generate polygon
        static Vertex verts[SINE_LUT_RESOLUTION]; // todo: same here, see if this can be static or a C-style array, to avoid unnecessary memory allocations
        for (size_t i = 0; i < SINE_LUT_RESOLUTION; i++) {
            verts[i] = Vertex{glm::vec3(points[i], depth), {0, 0}, color};
        }

        // Draw polygon
        draw_flat_polygon(transform, verts, SINE_LUT_RESOLUTION, anchor);
    }

    void Renderer::draw_box_solid(Transform transform, const glm::vec2 top_left, const glm::vec2 bottom_right, const glm::vec4 color, float depth, const AnchorPoint anchor) {
        // Derive corners of the box
        glm::vec2 tl = top_left;
        glm::vec2 br = bottom_right;
        glm::vec2 bl = { tl.x, br.y };
        glm::vec2 tr = { br.x, tl.y };

        // Create vertices
        Vertex verts[] = {
            { {tl, depth}, {1, 1}, color },
            { {tr, depth}, {1, 0}, color },
            { {br, depth}, {0, 1}, color },
            { {bl, depth}, {0, 0}, color }
        };
        draw_flat_polygon(transform, verts, 4, anchor);
    }

    void Renderer::draw_box_textured(Transform transform, const::std::string& texture, TextureType tex_type, const glm::vec2 top_left, const glm::vec2 bottom_right, const glm::vec4 color = { 1,1,1,1 }, float depth, AnchorPoint anchor) {
        get_texture(texture);

        // Derive corners of the box
        glm::vec2 tl = top_left;
        glm::vec2 br = bottom_right;
        glm::vec2 bl = { tl.x, br.y };
        glm::vec2 tr = { br.x, tl.y };

        // Create vertices
        if (tex_type == TextureType::stretch) {
            Vertex verts[4]{
                { {tl, depth}, {0, 1}, color * glm::vec4(1, 1, 1, 0) },
                { {tr, depth}, {1, 1}, color * glm::vec4(1, 1, 1, 0) },
                { {br, depth}, {1, 0}, color * glm::vec4(1, 1, 1, 0) },
                { {bl, depth}, {0, 0}, color * glm::vec4(1, 1, 1, 0) },
            };
            draw_polygon_textured(transform, verts, 4, texture, anchor);
        }
        else if (tex_type == TextureType::tile) {
            const glm::vec2 tc_multiplier = {
                (tr.x - tl.x) / _textures[texture].res.x,
                (bl.y - tl.y) / _textures[texture].res.y,
            };
            Vertex verts[4]{
                { {tl, depth}, glm::vec2{0, 0} *tc_multiplier, color * glm::vec4(1, 1, 1, 0) },
                { {tr, depth}, glm::vec2{1, 0} *tc_multiplier, color * glm::vec4(1, 1, 1, 0)},
                { {br, depth}, glm::vec2{1, -1} *tc_multiplier, color * glm::vec4(1, 1, 1, 0) },
                { {bl, depth}, glm::vec2{0, -1} *tc_multiplier, color * glm::vec4(1, 1, 1, 0) } ,
            };
            draw_polygon_textured(transform, verts, 4, texture, anchor);
        }
        else if (tex_type == TextureType::slice) {
            // Split into 9 segments
            constexpr float one_third = 1.0f / 3.0f;
            constexpr float two_third = 2.0f / 3.0f;
            const auto tres = glm::vec2(_textures[texture].res);
            // Top left
            Vertex verts[4];
            verts[0] = { {tl, depth}, {0, 0}, color * glm::vec4(1, 1, 1, 0)};
            verts[1] = { {tl + tres * glm::vec2(one_third, 0), depth}, {one_third, 0}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {tl + tres * glm::vec2(one_third, one_third), depth}, {one_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {tl + tres * glm::vec2(0, one_third), depth}, {0, one_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Top right
            verts[0] = { {tr + tres * glm::vec2(-one_third, 0), depth}, {two_third, 0}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {tr + tres * glm::vec2(0, 0), depth}, {1, 0}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {tr + tres * glm::vec2(0, one_third), depth}, {1, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {tr + tres * glm::vec2(-one_third, one_third), depth}, {two_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Bottom left
            verts[0] = { {bl + tres * glm::vec2(0, -one_third), depth}, {0, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {bl + tres * glm::vec2(one_third, -one_third), depth}, {one_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {bl + tres * glm::vec2(one_third, 0), depth}, {one_third, 1}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {bl + tres * glm::vec2(0, 0), depth}, {0, 1}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Bottom right
            verts[0] = { {br + tres * glm::vec2(-one_third, -one_third), depth}, {two_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {br + tres * glm::vec2(0, -one_third), depth}, {1, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {br + tres * glm::vec2(0, 0), depth}, {1, 1}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {br + tres * glm::vec2(-one_third, 0), depth}, {two_third, 1}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Top
            verts[0] = { {tl + tres * glm::vec2(one_third, 0), depth}, {one_third, 0}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {tr + tres * glm::vec2(-one_third, 0), depth}, {two_third, 0}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {tr + tres * glm::vec2(-one_third, one_third), depth}, {two_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {tl + tres * glm::vec2(one_third, one_third), depth}, {one_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Bottom
            verts[0] = { {bl + tres * glm::vec2(one_third, -one_third), depth}, {one_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {br + tres * glm::vec2(-one_third, -one_third), depth}, {two_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {br + tres * glm::vec2(-one_third, 0), depth}, {two_third, 1}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {bl + tres * glm::vec2(one_third, 0), depth}, {one_third, 1}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Left
            verts[0] = { {tl + tres * glm::vec2(0, one_third), depth}, {0, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {tl + tres * glm::vec2(one_third, one_third), depth}, {one_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {bl + tres * glm::vec2(one_third, -one_third), depth}, {one_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {bl + tres * glm::vec2(0, -one_third), depth}, {0, two_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Right
            verts[0] = { {tr + tres * glm::vec2(-one_third, one_third), depth}, {two_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {tr + tres * glm::vec2(0, one_third), depth}, {1, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {br + tres * glm::vec2(0, -one_third), depth}, {1, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {br + tres * glm::vec2(-one_third, -one_third), depth}, {two_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);

            // Middle
            verts[0] = { {tl + tres * glm::vec2(one_third, one_third), depth}, {one_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[1] = { {tr + tres * glm::vec2(-one_third, one_third), depth}, {two_third, one_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[2] = { {br + tres * glm::vec2(-one_third, -one_third), depth}, {two_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            verts[3] = { {bl + tres * glm::vec2(one_third, -one_third), depth}, {one_third, two_third}, color * glm::vec4(1, 1, 1, 0) };
            draw_polygon_textured(transform, verts, 4, texture, anchor);
        }

    }

    void Renderer::draw_flat_polygon(Transform transform, Vertex* verts, size_t n_verts, const AnchorPoint anchor) {
        // Scale to screen
        for (size_t i = 0; i < n_verts; i++) {
            verts[i].pos = pixels_to_normalized(verts[i].pos, anchor);
            verts[i].clip_rect = { apply_anchor_in_pixel_space(transform.top_left, transform.anchor), apply_anchor_in_pixel_space(transform.bottom_right, transform.anchor) };
        }

        // Add to render queue
        for (size_t i = 0; i < n_verts - 2; i++) {
            _flat_queue.push_back({ verts[0], verts[i + 2], verts[i + 1] });
        }
    }

    void Renderer::get_texture(const std::string& texture) {
        if (_textures.find(texture) == _textures.end()) {
            Texture tex;
            if (load_texture(texture, tex))
                _textures[texture] = tex;
            else
                printf("ERROR: Unable to find texture at path '%s'\n", texture.c_str());
        }
    }

    void Renderer::draw_polygon_textured(Transform transform, Vertex* verts, size_t n_verts, const std::string& texture, const AnchorPoint anchor) {
        // Upload texture if necessary
        get_texture(texture);

        // Scale to screen
        for (size_t i = 0; i < n_verts; ++i) {
            verts[i].pos = pixels_to_normalized(verts[i].pos, anchor);
            verts[i].clip_rect = { apply_anchor_in_pixel_space(transform.top_left, transform.anchor), apply_anchor_in_pixel_space(transform.bottom_right, transform.anchor) };
        }

        // Add to render queue
        for (size_t i = 0; i < n_verts - 2; i++) {
            _textured_queue[_textures[texture].id].push_back({verts[0], verts[i + 2], verts[i + 1]});
        }
    }

    void Renderer::init_luts() {
        // Init text LUT
        {
            // Only init if it's not yet initializdd
            if (!_wchar_lut.empty())
                return;

            // These are all just regular old ASCII, do those in bulk
            const std::wstring lut = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~°ø";
            for (size_t x = 0; x < lut.size(); x++) {
                _wchar_lut[lut[x]] = { static_cast<int>(x) };
            }

            // All the accented characters
            {
                const std::string ascii_list = "AAAAACEEEEIIIINOOOOOUUUUY"
                    "aaaaaceeeeiiiinooooouuuuy";
                const std::wstring accented_list = L"¡¿¬√ƒ«…» ÀÕÃŒœ—”“‘’÷⁄Ÿ€‹›"
                    "·‡‚„‰ÁÈËÍÎÌÏÓÔÒÛÚÙıˆ˙˘˚¸˝";
                const std::string accent_markers = "0135460134013450135401340"
                    "0135460134013450135401340";
                constexpr int accent_offset = 0x70 - '0';

                for (size_t x = 0; x < ascii_list.size(); x++) {
                    _wchar_lut[accented_list[x]] = { static_cast<int>(lut.find_first_of(ascii_list[x])), accent_markers[x] + accent_offset };
                }
            }
        }

        // Init sine LUT
        const float inv_sine_lut_size = 6.283185307179f / static_cast<float>(_sine_lut.size());
        for (size_t i = 0; i < _sine_lut.size(); i++) {
            const float x = static_cast<float>(i) * inv_sine_lut_size;
            _sine_lut[i] = sinf(x);
        }
    }

    void Renderer::draw_text(Transform transform, const std::wstring& text, glm::vec2 pos, glm::vec2 scale, glm::vec4 color, float depth, AnchorPoint ui_anchor, AnchorPoint text_anchor) {
        init_luts();
        glm::vec2 cur_pos = pos;

        // Calculate width
        float height = static_cast<float>(_font.grid_h) * scale.y;
        std::vector<float> widths;
        {
            float width = 0;
            for (auto& c : text) {
                if (c == '\n') {
                    widths.push_back(width);
                    width = 0;
                    continue;
                }

                std::vector<int>& wentry = _wchar_lut[static_cast<wchar_t>(c)];
                if (!wentry.empty())
                    width += static_cast<float>(_font.widths[wentry[0]]) * scale.x;
            }
            widths.push_back(width); // the final width doesn't get added in the for loop, do that manually
        }
        height *= static_cast<float>(widths.size());

        // Calculate offsets based on text anchor point
        std::vector<glm::vec3> offsets(widths.size());
        for (size_t i = 0; i < widths.size(); i++) {
            // Get the offset from the table
            glm::vec3 offset = glm::vec3(anchor_offsets[static_cast<size_t>(text_anchor)], 0);

            // We also need to invert the x-axis
            offset.x = -offset.x;

            // Since text by default renders from top_left, we need to offset the offset to the center
            offset.x -= 1;
            offset.y -= 1;
            
            // We need to scale this to the width and height of the 
            offset.x *= widths[i] * 0.5f;
            offset.y *= height * 0.5f;
            //offset.y -= 0.5f * static_cast<float>(height);

            // Add it to the offset array
            offsets[i] = offset;
            //offsets.push_back({0,0,0});
        }

        int width_idx = 0;
        for (auto& c : text) {
            // Handle newline
            if (c == '\n') {
                cur_pos.x = pos.x;
                cur_pos.y += static_cast<float>(_font.grid_h) * scale.y;
                width_idx++;
                continue;
            }
            if (c == '\r') {
                cur_pos.x = pos.x;
                continue;
            }

            // Create verts
            std::vector<int>& wentry = _wchar_lut[static_cast<wchar_t>(c)];

            for (size_t i = 0; i < wentry.size(); i++) {
                auto wc = wentry[i];
                glm::vec4 color_noalpha = color * glm::vec4(1, 1, 1, 0);
                glm::vec3 pos_depth = (glm::vec3(cur_pos, depth) + glm::vec3(0, i * 2, 0)) + offsets[width_idx];
                glm::vec2 off_uv = glm::vec2(wc % 16, wc >> 4) / glm::vec2(16.f, 8.f);
                glm::vec2 glyph_size = glm::vec2(1.f / 16.f, 1.f / 8.f);
                float grid_w_2 = static_cast<float>(_font.grid_w) * scale.x;
                float grid_h_2 = static_cast<float>(_font.grid_h) * scale.y;
                Vertex v1 = { // top left
                    pixels_to_normalized(pos_depth + glm::vec3(grid_w_2, 0, 0.0f), ui_anchor),
                    glm::vec2(1, 0)* glyph_size + off_uv,
                    color_noalpha,
                    {transform.top_left, transform.bottom_right}
                };
                Vertex v2 = { // top right
                    pixels_to_normalized(pos_depth + glm::vec3(0, 0, 0.0f), ui_anchor),
                    glm::vec2(0, 0) * glyph_size + off_uv,
                    color_noalpha,
                    {transform.top_left, transform.bottom_right}
                };
                Vertex v3 = { // bottom right
                    pixels_to_normalized(pos_depth + glm::vec3(0, grid_h_2, 0.0f), ui_anchor),
                    glm::vec2(0, 1) * glyph_size + off_uv,
                    color_noalpha,
                    {transform.top_left, transform.bottom_right}
                };
                Vertex v4 = { // bottom left
                    pixels_to_normalized(pos_depth + glm::vec3(grid_w_2, grid_h_2, 0.0f), ui_anchor),
                    glm::vec2(1, 1) * glyph_size + off_uv,
                    color_noalpha,
                    {transform.top_left, transform.bottom_right}
                };

                // Create triangles and add to queue
                _flat_queue.push_back({ v1, v3, v2 });
                _flat_queue.push_back({ v1, v4, v3 });
            }

            // Move cursor
            cur_pos.x += static_cast<float>(_font.widths[wentry[0]]) * scale.x;
        }
    }

    void Renderer::set_clipping_rectangle(bool enabled, glm::vec2 top_left, glm::vec2 bottom_right)
    {
        // If we don't want a clipping rectangle, we just set it to some ridiculously high range, it'll be fine
        if (enabled == false) {
            top_left = { -999999.f, -999999.f };
            bottom_right = { +999999.f, +999999.f };
            return;
        }

        // Otherwise yeah let's update the variable
        _clipping_rectangle = { top_left, bottom_right };
    }

    glm::vec2 Renderer::apply_anchor(const glm::vec2 pos, AnchorPoint anchor) {
        return pos + (glm::vec2(0.5f, -0.5f) * glm::vec2(_res) * (anchor_offsets[static_cast<size_t>(anchor)] + glm::vec2(1.0f, 1.0f)));
    }

    glm::vec2 Renderer::pixels_to_normalized(const glm::vec2 pos, AnchorPoint anchor) const {
        // Handle anchor and scale to window
        return glm::vec2( 2, -2 ) * (pos / glm::vec2(_res)) + anchor_offsets[static_cast<size_t>(anchor)];
    }

    glm::vec3 Renderer::pixels_to_normalized(const glm::vec3 pos, AnchorPoint anchor) const {
        // Handle anchor and scale to window
        return glm::vec3(2, -2, 1) * (pos / glm::vec3(_res, 1)) + glm::vec3(anchor_offsets[static_cast<size_t>(anchor)], 0.f);
    }

    glm::vec2 Renderer::apply_anchor_in_pixel_space(glm::vec2 pos, AnchorPoint anchor) const
    {
        glm::vec2 pixel_anchor_offsets[] = {
            {0.5f, 0.5f},
            {0.0f, 0.0f},
            {0.5f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 0.5f},
            {1.0f, 1.0f},
            {0.5f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.5f},
        };
        return pos + (pixel_anchor_offsets[static_cast<size_t>(anchor)] * glm::vec2(_res));
    }

    bool Renderer::load_texture(const std::string& path, Texture& handle) {
        // Load image
        int w, h, c;
        uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 4);

        // If not on disk, find in resources
        if (!data) {
            int size;
            char* data_resource;
            read_resource(path, size, data_resource, L"PNG", dll);
            data = stbi_load_from_memory((stbi_uc*)data_resource, size, &w, &h, &c, 4);
        }

        // Did it load correctly?
        if (!data || !w || !h) {
            printf("ERROR: Failed to load texture '%s'! STBI returned the following error: %s", path.c_str(), stbi_failure_reason());
            return false;
        }

        // Upload texture to GPU
        glGenTextures(1, &handle.id);
        glBindTexture(GL_TEXTURE_2D, handle.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Set type
        handle.res = { w, h };

        // Clean up
        STBI_FREE(data);
        return true;
    }

    bool Renderer::load_font(const std::string& path) {
        // Load image
        int w, h, c;
        uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 4);

        // If failed from disk, try from resources
        if (!data) {
            int size;
            char* data_resource;
            read_resource(path, size, data_resource, L"PNG", dll);
            data = stbi_load_from_memory((stbi_uc*)data_resource, size, &w, &h, &c, 4);
        }
        
        // If we still don't have an image, throw an error
        if (!data) {
            printf("ERROR: File not found: \"%s\".", path.c_str());
            return false;
        }

        // Make sure it's perfectly divides into a 16x16 grid
        if (w % 16 != 0 || h % 8 != 0) {
            printf("ERROR: Font atlas is not a multiple of 16 in size. Probably bad size.");
            return false;
        }

        // Calculate glyph size
        const glm::ivec2 glyph_size = { w / 16, h / 8 };

        // Calculate widths
        int widths[128]{ glyph_size.x };

        // For each glyph get the width by scanning for a red pixel
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 16; x++) {
                int width = glyph_size.x;
                for (int glyph_x = 0; glyph_x < glyph_size.x; glyph_x++) {
                    // Get pixel
                    const int sample_x = (x * glyph_size.x) + glyph_x;
                    const int sample_y = y * glyph_size.y;
                    const uint32_t pixel = (reinterpret_cast<uint32_t*>(data))[sample_y * w + sample_x];

                    // If red (127, 0, 0), the current x-coordinate is the glyph width
                    if ((pixel & 0x00FFFFFF) == 0x00007F) {
                        width = glyph_x + 2;
                        break;
                    }
                }
                widths[x + (y * 16)] = width;
            }
        }

        // Remove all the red pixels from the font
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t& pixel = (reinterpret_cast<uint32_t*>(data))[y * w + x];
                if ((pixel & 0x00FFFFFF) == 0x00007F) {
                    pixel = 0;
                }
            }
        }

        // Upload font texture to GPU
        GLuint texture_gpu;
        glGenTextures(1, &texture_gpu);
        glBindTexture(GL_TEXTURE_2D, texture_gpu);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Create font object
        std::vector<int> widths_vector(128);
        memcpy_s(widths_vector.data(), widths_vector.size() * 4, widths, 128ull * 4ull);
        _font = Font {
            texture_gpu,
            static_cast<uint16_t>(glyph_size.x),
            static_cast<uint16_t>(glyph_size.y),
            widths_vector
        };

        return true;
    }

    GLuint Renderer::shader_from_file(const std::string& path) {
        const GLuint shader_gpu = glCreateProgram();

        bool vert_loaded = shader_part_from_file(path + ".vert", ShaderType::vertex, shader_gpu);
        bool frag_loaded = shader_part_from_file(path + ".frag", ShaderType::pixel, shader_gpu);
        bool comp_loaded = shader_part_from_file(path + ".comp", ShaderType::compute, shader_gpu);
        shader_part_from_file(path + ".geom", ShaderType::geometry, shader_gpu);

        if (
            (vert_loaded && frag_loaded) == false &&
            comp_loaded == false)
        {
            printf("[ERROR] Failed to load shader '%s'! Either the shader files do not exist, or a compilation error occurred.\n", path.c_str());
        }
        if (
            (vert_loaded || frag_loaded) == true &&
            comp_loaded == true)
        {
            printf("[ERROR] Failed to load shader '%s'! Unsure whether to use vertex/fragment shaders, or compute shaders, since both exist.\n", path.c_str());
        }

        glLinkProgram(shader_gpu);

        return shader_gpu;
    }

    GLuint Renderer::shader_from_resource(const std::string& path)
    {
        const GLuint shader_gpu = glCreateProgram();

        bool vert_loaded = shader_part_from_resource(path + ".vert", ShaderType::vertex, shader_gpu);
        bool frag_loaded = shader_part_from_resource(path + ".frag", ShaderType::pixel, shader_gpu);
        bool comp_loaded = shader_part_from_resource(path + ".comp", ShaderType::compute, shader_gpu);
        shader_part_from_resource(path + ".geom", ShaderType::geometry, shader_gpu);

        if (
            (vert_loaded && frag_loaded) == false &&
            comp_loaded == false)
        {
            printf("[ERROR] Failed to load shader! Either the shader files do not exist, or a compilation error occurred.\n");
        }
        if (
            (vert_loaded || frag_loaded) == true &&
            comp_loaded == true)
        {
            printf("[ERROR] Failed to load shader! Unsure whether to use vertex/fragment shaders, or compute shaders, since both exist.\n");
        }

        glLinkProgram(shader_gpu);

        return shader_gpu;
    }

    bool Renderer::shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program) {
        constexpr int shader_types[]
        {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            GL_GEOMETRY_SHADER,
            GL_COMPUTE_SHADER,
        };

        // Read shader source file
        int shader_size = 0;
        char* shader_data = nullptr;
        read_file(path, shader_size, shader_data);

        if (shader_data == nullptr) {
            return false;
        }
        
        // Create shader on GPU
        const GLuint type_to_create = shader_types[static_cast<int>(type)];
        const GLuint shader = glCreateShader(type_to_create);

        // Compile shader source
        const char* data = shader_data;
        glShaderSource(shader, 1, &data, &shader_size);
        glCompileShader(shader);

        // Error checking
        GLint result = GL_FALSE;
        int log_length;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> frag_shader_error(log_length > 1 ? log_length : 1);
        glGetShaderInfoLog(shader, log_length, nullptr, frag_shader_error.data());
        if (log_length > 0)
        {
            printf("[ERROR] File '%s':\n\n%s\n", path.c_str(), frag_shader_error.data());
            return false;
        }

        // Attach to program
        glAttachShader(program, shader);

        return true;
    }

    bool Renderer::shader_part_from_resource(const std::string& name, ShaderType type, const GLuint& program)
    {
        constexpr int shader_types[]
        {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            GL_GEOMETRY_SHADER,
            GL_COMPUTE_SHADER,
        };

        // Read shader source file
        int shader_size = 0;
        char* shader_data = nullptr;
        read_resource(name, shader_size, shader_data, L"ShaderSource", dll);

        if (shader_data == nullptr) {
            return false;
        }

        // Create shader on GPU
        const GLuint type_to_create = shader_types[static_cast<int>(type)];
        const GLuint shader = glCreateShader(type_to_create);

        // Compile shader source
        const char* data = shader_data;
        glShaderSource(shader, 1, &data, &shader_size);
        glCompileShader(shader);

        // Error checking
        GLint result = GL_FALSE;
        int log_length;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> frag_shader_error(log_length > 1 ? log_length : 1);
        glGetShaderInfoLog(shader, log_length, nullptr, frag_shader_error.data());
        if (log_length > 0)
        {
            printf("[ERROR] \n%s\n", frag_shader_error.data());
            return false;
        }

        // Attach to program
        glAttachShader(program, shader);

        return true;
    }

    bool Renderer::shader_part_from_string(const std::string& string, ShaderType type, const GLuint& program) {
        constexpr int shader_types[]
        {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            GL_GEOMETRY_SHADER,
            GL_COMPUTE_SHADER,
        };

        // Read shader source file
        const int shader_size = static_cast<int>(string.size());
        const char* shader_data = string.c_str();

        if (shader_data == nullptr) {
            return false;
        }

        // Create shader on GPU
        const GLuint type_to_create = shader_types[static_cast<int>(type)];
        const GLuint shader = glCreateShader(type_to_create);

        // Compile shader source
        const char* data = shader_data;
        glShaderSource(shader, 1, &data, &shader_size);
        glCompileShader(shader);

        // Error checking
        GLint result = GL_FALSE;
        int log_length;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> frag_shader_error(log_length > 1 ? log_length : 1);
        glGetShaderInfoLog(shader, log_length, nullptr, frag_shader_error.data());
        if (log_length > 0)
        {
            printf("[ERROR] Shader from string:\n\n%s\n", frag_shader_error.data());
            return false;
        }

        // Attach to program
        glAttachShader(program, shader);

        return true;
    }
}
