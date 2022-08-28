#include "Renderer.h"

#include <fstream>
#include <GL/gl3w.h>

#include "glm/geometric.hpp"
#include "glm/gtx/exterior_product.hpp"


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

    void Renderer::init() {
        // Init + window settings
        glfwInit();
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
            exit(1);
        }
        printf("INFO: GL_VERSION returned %s\n", (char*)glGetString(GL_VERSION));
        glDebugMessageCallback(debug_callback_func, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        init(window);
    }

    void Renderer::init(GLFWwindow* window) {
        // Init basic rendering
        _window = window;
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glfwSwapInterval(0);
        //shader = shader_from_file("Shaders/sprite");
        shader = shader_from_string(vert_shader, frag_shader);
        glUseProgram(shader);

        // Create vertex buffer
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Setup vertex array
        glVertexAttribPointer(0, sizeof(Vertex::pos  ) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos  .x)));
        glVertexAttribPointer(1, sizeof(Vertex::tc   ) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tc   .x)));
        glVertexAttribPointer(2, sizeof(Vertex::color) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color.x)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
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

        // Clear queues
        _line_queue.clear();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::gl_error() {
        auto error = glGetError();
        if (error) {
            printf("ERROR: %i\n", error);
        }
    }

    void Renderer::end_frame() {
        // Render line queue
        glUseProgram(shader);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Triangle) * _line_queue.size()), &_line_queue[0], GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, _line_queue.size() * 3);
        flip_buffers();
    }

    void Renderer::flip_buffers() {
        glfwSwapBuffers(_window);
    }

    void Renderer::draw_line(glm::vec2 a, glm::vec2 b, glm::vec4 color, float width, float depth) {
        Vertex v1, v2, v3, v4;

        // Calculate normal
        glm::vec2 normal = (b - a);
        normal = { -normal.y, normal.x };
        normal = glm::normalize(normal) * width;

        // Create triangles
        v1.pos = glm::vec3(a - normal, depth) / glm::vec3(_res, 1.0f) ;
        v2.pos = glm::vec3(b - normal, depth) / glm::vec3(_res, 1.0f) ;
        v3.pos = glm::vec3(b + normal, depth) / glm::vec3(_res, 1.0f) ;
        v4.pos = glm::vec3(a + normal, depth) / glm::vec3(_res, 1.0f) ;
        v1.tc = { 0, 0 };
        v2.tc = { 0, 0 };
        v3.tc = { 0, 0 };
        v4.tc = { 0, 0 };
        v1.color = color;
        v2.color = color;
        v3.color = color;
        v4.color = color;
        _line_queue.push_back({v1, v3, v2});
        _line_queue.push_back({v1, v4, v3});
    }

    void Renderer::draw_linebox(const glm::vec2 top_left, const glm::vec2 bottom_right, const glm::vec4 color, const float width, const float depth) {
        // Derive corners of the box
        glm::vec2 tl = top_left;
        glm::vec2 br = bottom_right;
        glm::vec2 bl = { tl.x, br.y };
        glm::vec2 tr = { br.x, tl.y };

        // Draw the lines
        glm::vec2 _x = { width, 0 };
        glm::vec2 _y = { _x.y, _x.x };
        draw_line(tl - _x, tr + _x, color, width, depth);
        draw_line(tr + _y, br - _y, color, width, depth);
        draw_line(br + _x, bl - _x, color, width, depth);
        draw_line(bl - _y, tl + _y, color, width, depth);
    }

    void Renderer::draw_linecircle(glm::vec2 center, glm::vec2 scale, glm::vec4 color, float width, float depth) {
        constexpr int resolution = 32;
        std::vector<glm::vec2> points(resolution);

        // Generate points
        for (int i = 0; i < resolution; i++) {
            float t = (float)i / (float)resolution * 6.28318530718f;
            points[i] = center + (scale * glm::vec2{ cosf(t), sinf(t) });
        }

        // Draw lines
        for (int i = 0; i < resolution; i++) {
            draw_line(points[i], points[(i + 1) % resolution], color, width, depth);
        }
    }

    GLuint Renderer::shader_from_file(std::string path) {
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

    GLuint Renderer::shader_from_string(std::string vert, std::string frag) {
        const GLuint shader_gpu = glCreateProgram();
        shader_part_from_string(vert, ShaderType::vertex, shader_gpu);
        shader_part_from_string(frag, ShaderType::pixel, shader_gpu);
        glLinkProgram(shader_gpu);
        return shader_gpu;
    }

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

    bool Renderer::shader_part_from_file(const std::string& path, ShaderType type, const GLuint& program) {
        constexpr int shader_types[]
        {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            GL_GEOMETRY_SHADER,
            GL_COMPUTE_SHADER,
        };

        //Read shader source file
        int shader_size = 0;
        char* shader_data = nullptr;
        read_file(path, shader_size, shader_data);

        if (shader_data == nullptr) {
            return false;
        }
        
        //Create shader on GPU
        const GLuint type_to_create = shader_types[static_cast<int>(type)];
        const GLuint shader = glCreateShader(type_to_create);

        //Compile shader source
        const char* data = shader_data;
        glShaderSource(shader, 1, &data, &shader_size);
        glCompileShader(shader);

        //Error checking
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

        //Attach to program
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

        //Read shader source file
        int shader_size = string.size();
        const char* shader_data = string.c_str();

        if (shader_data == nullptr) {
            return false;
        }

        //Create shader on GPU
        const GLuint type_to_create = shader_types[static_cast<int>(type)];
        const GLuint shader = glCreateShader(type_to_create);

        //Compile shader source
        const char* data = shader_data;
        glShaderSource(shader, 1, &data, &shader_size);
        glCompileShader(shader);

        //Error checking
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

        //Attach to program
        glAttachShader(program, shader);

        return true;
    }
}
