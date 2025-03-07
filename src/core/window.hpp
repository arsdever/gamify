#pragma once

#include <glm/fwd.hpp>

#include "core/core_fwd.hpp"

#include "common/event.hpp"
#include "core/window_events.hpp"

namespace core
{

/**
 * @brief Class representing each window in the engine.
 *
 * The window class itself represents a physical window that is open and
 * running.
 */
class window : public std::enable_shared_from_this<window>
{
public:
    window();
    ~window();

    void init();
    void activate();

    void set_title(std::string_view title);
    std::string get_title() const;

    size_t get_width() const;
    size_t get_height() const;
    glm::uvec2 get_size() const;
    void resize(size_t width, size_t height);

    glm::uvec2 get_position() const;
    void set_position(size_t x, size_t y);
    void move(size_t dx, size_t dy);

    void update();

    void set_as_input_source(bool flag = true);
    bool get_is_input_source() const;

    void set_can_grab(bool flag = true);
    bool get_can_grab() const;

    void grab_mouse(bool flag = true);
    bool get_has_grab() const;

    void add_viewport(std::shared_ptr<viewport> vp);
    void remove_viewport(std::shared_ptr<viewport> vp);
    std::vector<std::shared_ptr<viewport>> get_viewports();

    std::shared_ptr<window_events> get_events() const;

    event<void(std::shared_ptr<window>)> on_user_initialize;

    static std::shared_ptr<window> get_main_window();

    void* get_native_handle() const;

private:
    void setup_mouse_callbacks();

    void setup_mouse_enter_callback();
    void setup_mouse_move_callback();
    void setup_mouse_button_callback();
    void setup_mouse_wheel_callback();

    void check_for_drag();

    void configure_input_system();

private:
    struct window_private_data;
    std::unique_ptr<window_private_data> _p { nullptr };

    static std::shared_ptr<window> _main_window;
};

} // namespace core
