#include "../resource.hpp"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#define GLFW_INCLUDE_NONE
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <fstream>
#include <memory>
#include "../../common.hpp"
#include "../../input.hpp"
#include "../../log.hpp"
#include "device_software.hpp"

namespace Gfx {
    static void glfw_error_callback(int error, const char* description) { LOG(Error, "Error %i: %s", error, description); }

    DeviceSoftware::DeviceSoftware(int width, int height, const char* window_title) {
        if (!glfwInit()) {
            LOG(Fatal, "Failed to initialize GLFW");
            exit(1);
        }

        glfwSetErrorCallback(glfw_error_callback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        window = glfwCreateWindow(width, height, window_title, NULL, NULL);
        if (!window) {
            LOG(Fatal, "Failed to create GLFW window");
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        // input_setup();
    }

    void DeviceSoftware::handle_resize(int width, int height) {}

    void DeviceSoftware::get_window_size(int& width, int& height) { glfwGetFramebufferSize(window, &width, &height); }

    bool DeviceSoftware::should_stay_open() { return !glfwWindowShouldClose(window); }

    void DeviceSoftware::set_full_screen(bool full_screen) { 
        // todo(device_software_set_full_screen): desc: DeviceSoftware::set_full_screen(...)
        (void)full_screen; 
        TODO(); 
    }

    void DeviceSoftware::begin_frame() {
        // Calculate delta time
        static double prev_time = 0.0;
        static double curr_time = 0.0;
        prev_time               = curr_time;
        curr_time               = glfwGetTime();
        delta_time              = curr_time - prev_time;

        // Any frametime above 1.0 seconds (or less than 1 fps) is spicy, so we should limit the deltatime
        if (delta_time > 1.0) {
            delta_time = 1.0;
        }

        glfwPollEvents();

        // todo(device_software_begin_frame): desc: DeviceSoftware::begin_frame(...)
    }

    void DeviceSoftware::end_frame() {
        glfwSwapBuffers(window);

        auto incoming_input_data = Input::get_ptr_incoming();
        if (incoming_input_data) {
            if (incoming_input_data->move_mouse_mode != Input::MoveMouseMode::None) {
                double pos_x = 0.0;
                double pos_y = 0.0;

                if (incoming_input_data->move_mouse_mode == Input::MoveMouseMode::Relative) {
                    glfwGetCursorPos(this->window, &pos_x, &pos_y);
                }

                glfwSetCursorPos(
                    this->window, pos_x + (double)incoming_input_data->mouse_move_amount.x,
                    pos_y + (double)incoming_input_data->mouse_move_amount.y);

                incoming_input_data->mouse_move_amount = glm::vec2(0.0f);
                incoming_input_data->move_mouse_mode   = Input::MoveMouseMode::None;
            }
        };

        if (queue_mouse_visible) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            queue_mouse_visible = false;
        }

        if (desired_cursor_mode != curr_cursor_mode) {
            glfwSetCursor(this->window, this->cursors[(size_t)desired_cursor_mode]);
            curr_cursor_mode = desired_cursor_mode;
        }

        // todo(device_software_end_frame): desc: DeviceSoftware::end_frame(...)
    }

    void DeviceSoftware::clear_framebuffer(const ClearParams& clear_params) {
        // todo(device_software_clear_framebuffer): desc: DeviceSoftware::clear_framebuffer(...)
    }

    void DeviceSoftware::blit_pixels(ResourceID src, ResourceID dest, glm::ivec2 size, glm::ivec2 dest_tl, glm::ivec2 src_tl) {
        int w, h;
        get_window_size(w, h);

        // todo(device_software_blit_pixels): desc: DeviceSoftware::blit_pixels(...)
    }

    void DeviceSoftware::set_camera(const Transform& transform) { (void)transform; TODO(); }

    void DeviceSoftware::set_clip_rect(glm::ivec2 top_left, glm::ivec2 size) {
        int w, h;
        if (this->active_framebuffer.is_valid() == false) {
            this->get_window_size(w, h);
        } else {
            auto* resource = (TextureResource*)(resources.at(this->active_framebuffer.id));
            w              = resource->width;
            h              = resource->height;
        }
        
        // todo(device_software_set_clip_rect): desc: DeviceSoftware::set_clip_rect(...)
    }

    void DeviceSoftware::set_viewport(glm::ivec2 top_left, glm::ivec2 size) {
        int w, h;
        if (this->active_framebuffer.is_valid() == false) {
            this->get_window_size(w, h);
        } else {
            auto* resource = (TextureResource*)(resources.at(this->active_framebuffer.id));
            w              = resource->width;
            h              = resource->height;
        }
        // todo(device_software_set_viewport): desc: DeviceSoftware::set_viewport(...)
    }

    void DeviceSoftware::set_render_target(ResourceID render_target) {
        // todo(device_software_set_render_target): desc: DeviceSoftware::set_render_target(...)
    }

    float DeviceSoftware::get_delta_time() { return (float)delta_time; }

    ResourceID DeviceSoftware::load_pipeline_raster(const char* shader_vs, const char* shader_ps) {
        // todo(device_software_load_pipeline_raster): desc: DeviceSoftware::load_pipeline_raster(...)
        return ResourceID::invalid();
    }

    void DeviceSoftware::begin_raster_pass(const ResourceID raster_pipeline) {
        render_pass_active = raster_pipeline;
        // auto pipeline      = resources.at(raster_pipeline.id); 
        // todo(device_software_begin_raster_pass): desc: DeviceSoftware::begin_raster_pass(...)
    }

    void DeviceSoftware::end_raster_pass() {
        render_pass_active = ResourceID::invalid();
        // todo(device_software_end_raster_pass): desc: DeviceSoftware::end_raster_pass(...)
    }

    void DeviceSoftware::execute_raster(const size_t n_vertices, const int vertex_offset) {
        // todo(device_software_execute_raster): desc: DeviceSoftware::execute_raster(...)
    }

    void DeviceSoftware::set_constants(const std::vector<uint32_t>& constants) {
        // todo(device_software_set_constants): desc: DeviceSoftware::set_constants(...)
    }

    void DeviceSoftware::set_view_offset(const glm::vec2& offset) {
        view_offset = offset;
    }

    void DeviceSoftware::set_view_scale(const glm::vec2& scale) {
        view_scale = scale;
    }

    Resource* DeviceSoftware::get_resource(const ResourceID id) {
        return (Resource*)(&resources.at(id.id));
    }

    void DeviceSoftware::delete_resource(const ResourceID resource_to_destroy) {
        // todo(device_software_delete_resource): desc: DeviceSoftware::delete_resource(...)
    }

    void DeviceSoftware::bind_resources(const std::vector<ResourceWithOffset>& bindings) {
        // todo(device_software_bind_resources): desc: DeviceSoftware::bind_resources(...)
    }

    ResourceID DeviceSoftware::create_buffer(const std::string_view& name, const size_t size_bytes, const void* data) {
        // todo(device_software_create_buffer): desc: DeviceSoftware::create_buffer(...)
        return ResourceID::invalid();
    }

    void DeviceSoftware::upload_data_to_buffer(
        const ResourceID buffer, const size_t offset_bytes, const size_t size_bytes, const void* data) {
        // auto buffer_resource = (BufferResource*)(resources.at(buffer.id));
        // if ((offset_bytes + size_bytes) > buffer_resource->size) {
        //     LOG(Warning, "GPU buffer data upload overflow! Data will not be copied.");
        //     return;
        // }
        // todo(device_software_upload_data_to_buffer): desc: DeviceSoftware::upload_data_to_buffer(...)
    }

    ResourceID DeviceSoftware::create_texture(
        const glm::ivec3 resolution, const TextureType type, const PixelFormat format, const void* data,
        const bool is_framebuffer) {
        assert((resolution.x > 0 && resolution.y > 0 && resolution.z > 0) && "Attempt to create texture with size 0!");
        assert(type != TextureType::Invalid && "Attempt to create GPU texture with invalid type!");
        assert(format != PixelFormat::Invalid && "Attempt to create GPU texture with invalid format!");

        // todo(device_software_create_texture): desc: DeviceSoftware::create_texture(...)
        return ResourceID::invalid();
    }

    void DeviceSoftware::bind_texture(int slot, const ResourceID texture) {
        // todo(device_software_bind_texture): desc: DeviceSoftware::bind_texture(...)
    }

    void DeviceSoftware::resize_texture(ResourceID id, glm::ivec3 new_resolution) {
        // todo(device_software_resize_texture): desc: DeviceSoftware::resize_texture(...)
    }

    Input::Key glfw_to_key(int key) {
        // A to Z
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            return (Input::Key)(key - GLFW_KEY_A + (int)Input::Key::A);
        }
        // 0 to 9
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            return (Input::Key)(key - GLFW_KEY_0 + (int)Input::Key::_0);
        }
        // F1 to F12
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12) {
            return (Input::Key)(key - GLFW_KEY_F1 + (int)Input::Key::F1);
        }
        // Misc
        if (key == GLFW_KEY_SPACE) return Input::Key::Space;
        if (key == GLFW_KEY_ESCAPE) return Input::Key::Escape;
        if (key == GLFW_KEY_ENTER) return Input::Key::Enter;
        if (key == GLFW_KEY_TAB) return Input::Key::Tab;
        if (key == GLFW_KEY_LEFT_SHIFT) return Input::Key::LeftShift;
        if (key == GLFW_KEY_LEFT_CONTROL) return Input::Key::LeftControl;
        if (key == GLFW_KEY_LEFT_ALT) return Input::Key::LeftAlt;
        if (key == GLFW_KEY_RIGHT_SHIFT) return Input::Key::RightShift;
        if (key == GLFW_KEY_RIGHT_CONTROL) return Input::Key::RightControl;
        if (key == GLFW_KEY_RIGHT_ALT) return Input::Key::RightAlt;
        if (key == GLFW_KEY_UP) return Input::Key::Up;
        if (key == GLFW_KEY_DOWN) return Input::Key::Down;
        if (key == GLFW_KEY_LEFT) return Input::Key::Left;
        if (key == GLFW_KEY_RIGHT) return Input::Key::Right;
        return Input::Key::Invalid;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)window;
        (void)scancode;
        (void)mods;
        auto incoming_input_data = Input::get_ptr_incoming();
        if (!incoming_input_data) return;

        switch (action) {
        case GLFW_PRESS: incoming_input_data->keys[(size_t)glfw_to_key(key)] = true; break;
        case GLFW_RELEASE: incoming_input_data->keys[(size_t)glfw_to_key(key)] = false; break;
        default: break;
        }
    }

    static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
        (void)window;
        auto incoming_input_data = Input::get_ptr_incoming();
        if (!incoming_input_data) return;

        incoming_input_data->mouse_x = xpos;
        incoming_input_data->mouse_y = ypos;
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        (void)window;
        (void)mods;
        auto incoming_input_data = Input::get_ptr_incoming();
        if (!incoming_input_data) return;

        Input::MouseButton mouse_button = Input::MouseButton::Left;
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: mouse_button = Input::MouseButton::Left; break;
        case GLFW_MOUSE_BUTTON_RIGHT: mouse_button = Input::MouseButton::Right; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: mouse_button = Input::MouseButton::Middle; break;
        }

        switch (action) {
        case GLFW_PRESS: incoming_input_data->mouse_buttons[(size_t)mouse_button] = true; break;
        case GLFW_RELEASE: incoming_input_data->mouse_buttons[(size_t)mouse_button] = false; break;
        }
    }

    static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        (void)window;
        auto incoming_input_data = Input::get_ptr_incoming();
        if (!incoming_input_data) return;

        incoming_input_data->mouse_scroll_x += xoffset;
        incoming_input_data->mouse_scroll_y += yoffset;
    }

    void DeviceSoftware::input_setup() {
        this->input_data = Device::fetch_incoming_input_data_pointer();
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
    }

    void DeviceSoftware::set_mouse_visible(bool visible) {
        if (!visible) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else {
            this->queue_mouse_visible = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
    }

    void DeviceSoftware::set_cursor_mode(Gfx::CursorMode cursor_mode) {
        this->desired_cursor_mode = cursor_mode;
    }

    PairResourceID DeviceSoftware::allocate_resource_slot(const ResourceType type) {
        // Reuse previously deallocated slots
        if (recycled_resource_slots.empty() == false) {
            const ResourceID slot = {.id = recycled_resource_slots.front(), .type = (uint32_t)type};
            recycled_resource_slots.pop_front();
            return PairResourceID{
                .resource = resources.at(slot.id),
                .id       = slot,
            };
        }

        // If there aren't any, allocate a new one
        const ResourceID slot = {.id = (uint32_t)resources.size(), .type = (uint32_t)type};
        resources.push_back(nullptr);
        return PairResourceID{
            .resource = resources.at(slot.id),
            .id       = slot,
        };
    }

    glm::vec2 DeviceSoftware::get_view_scale() {
      return view_scale;
    }

    std::vector<PixelRGBA_8> DeviceSoftware::get_window_framebuffer() {
        return std::vector<PixelRGBA_8>();
    }

    void DeviceSoftware::set_multisample(bool enable) {
        // todo(device_software_set_multisample): desc: DeviceSoftware::set_multisample(...)
    }

    ResourceID DeviceSoftware::load_pipeline_raster() {
        return ResourceID::invalid();
    }
} // namespace Gfx
