#pragma once
#include "panel.hpp"

namespace UI {
    constexpr float NOODLE_SPEED_SLOW = 3.0f;
    constexpr float NOODLE_SPEED_FAST = 256.0f;

    struct PanelCreateInfo {
        std::string title = "Yippee panels have names now :3";
        glm::vec2 top_left;
        glm::vec2 size = {128.0f, 128.0f};
        glm::vec2 min_size = {128.0f, 128.0f};
        glm::vec2 max_size = {99999.0f, 99999.0f};
        glm::vec4 bg_color = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
        bool maximized     = false;
    };

    enum class PinType {
        None = 0,
        Midi,
        Audio,
    };

    enum class PinDirection {
        None = 0,
        Input,
        Output,
    };

    struct Pin {
        PinType type;
        PinDirection direction;
        size_t panel_id; // what panel this pin belongs to
        std::string pin_id;// name index into Panel::pins
        float position_y; // what vertical position to attach the pin on the panel, relative to the top of the panel content
        glm::vec2 noodle_pos; // relative to panel top left
        glm::vec2 noodle_target; // where does the noodle want to be
        float noodle_speed = NOODLE_SPEED_SLOW;
        bool over_noodle;
        bool over_pin;
        bool being_dragged;
        glm::vec2 drag_mouse_offset;
    };
    
    struct Noodle {
        Pin source;
        Pin destination;
    };

    std::vector<Panel*>& get_panels_in_order();
    const std::vector<Noodle> get_noodles();
    size_t new_panel(const PanelCreateInfo& panel_create_info);
    size_t load_panel(const char* path, const glm::vec2 top_left = {0.0f, 0.0f});
    Panel& get_panel(const size_t id);
    void panel_input();
    void panel_render();
} // namespace UI
