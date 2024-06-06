/* clang-format off */
#include <glad/gl.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#include <glm/ext.hpp>
#include <prof/profiler.hpp>

#include "experimental/window.hpp"

#include "experimental/editor_window.hpp"
#include "experimental/window_events.hpp"
#include "input_system.hpp"
#include "logging.hpp"

namespace
{
logger log() { return get_logger("window"); }

experimental::mouse_event::modifiers
glfw_keyboard_modifiers_to_mouse_event_modifiers(int mods)
{
    experimental::mouse_event::modifiers modifiers;
    modifiers = static_cast<experimental::mouse_event::modifiers>(mods);
    return modifiers;
}

static constexpr std::chrono::milliseconds MAX_CLICK_REACTION_TIME { 300 };
static constexpr double DRAG_START_DISTANCE { 5.0 };

} // namespace

namespace experimental
{

struct window::window_private_data
{
    GLFWwindow* _glfw_window_handle { nullptr };
    glm::uvec2 _size { 800, 600 };
    glm::uvec2 _position { 200, 200 };
    window_events _events;
    std::vector<std::shared_ptr<viewport>> _viewports;
    std::string _title { "Window" };
    bool _can_grab { false };
    bool _has_grab { false };
    bool _is_input_source { false };

    struct
    {
        int _buttons;
        mouse_event::modifiers _mods;
        glm::vec2 _position;
        glm::vec2 _press_started_position;
        std::chrono::steady_clock::time_point _last_click_time;
        std::chrono::steady_clock::time_point _last_double_click_time;
        bool _is_drag;
    } _mouse_state;
};

window::window() { _p = std::make_unique<window_private_data>(); }

window::~window()
{
    glfwDestroyWindow(_p->_glfw_window_handle);
    _p = nullptr;
}

void window::init()
{
    if (_p->_glfw_window_handle != nullptr)
    {
        log()->error("The window is already initialized");
        return;
    }

    std::string title = _p->_title;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _p->_glfw_window_handle =
        glfwCreateWindow(static_cast<int>(get_width()),
                         static_cast<int>(get_height()),
                         title.c_str(),
                         nullptr,
                         editor_window::get()->_p->_glfw_window_handle);
    if (_p->_glfw_window_handle == nullptr)
    {
        log()->error("Failed to create GLFW window");
        return;
    }

    glfwSetWindowPos(_p->_glfw_window_handle, _p->_position.x, _p->_position.y);

    glfwSetWindowUserPointer(_p->_glfw_window_handle, this);

    glfwSetWindowSizeCallback(_p->_glfw_window_handle,
                              [](GLFWwindow* wnd, int w, int h)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        glm::uvec2 old_size = _this->_p->_size;
        _this->_p->_size = { w, h };
        _this->get_events().resize(
            resize_event(old_size, _this->_p->_size, _this));
    });

    glfwSetWindowPosCallback(_p->_glfw_window_handle,
                             [](GLFWwindow* wnd, int xpos, int ypos)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        glm::uvec2 old_pos = _this->_p->_position;
        _this->_p->_position = { xpos, ypos };
        _this->get_events().move(
            move_event(old_pos, _this->_p->_position, _this));
    });

    activate();

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        log()->error("Failed to initialize GLAD");
        return;
    }

    on_user_initialize(shared_from_this());
    configure_input_system();
}

void window::activate() { glfwMakeContextCurrent(_p->_glfw_window_handle); }

void window::set_title(std::string_view title)
{
    _p->_title = title;
    if (_p->_glfw_window_handle)
    {
        glfwSetWindowTitle(_p->_glfw_window_handle, title.data());
    }
}

std::string window::get_title() const { return std::string(_p->_title); }

size_t window::get_width() const { return _p->_size.x; }

size_t window::get_height() const { return _p->_size.y; }

glm::uvec2 window::get_size() const { return _p->_size; }

void window::resize(size_t width, size_t height)
{
    if (_p->_glfw_window_handle)
    {
        glfwSetWindowSize(_p->_glfw_window_handle,
                          static_cast<int>(width),
                          static_cast<int>(height));
    }
    else
    {
        _p->_size = { width, height };
    }
}

void window::set_position(size_t x, size_t y) { move(x, y); }

glm::uvec2 window::get_position() const { return _p->_position; }

void window::move(size_t x, size_t y)
{
    if (_p->_glfw_window_handle)
    {
        glfwSetWindowPos(
            _p->_glfw_window_handle, static_cast<int>(x), static_cast<int>(y));
    }
    else
    {
        _p->_position = { x, y };
    }
}

void window::update()
{
    if (_p->_glfw_window_handle == nullptr)
    {
        return;
    }

    if (glfwWindowShouldClose(_p->_glfw_window_handle))
    {
        get_events().close(close_event(this));
        glfwDestroyWindow(_p->_glfw_window_handle);
        _p->_glfw_window_handle = nullptr;
        return;
    }

    if (input_system::is_key_down(345) && input_system::is_key_down(346))
    {
        glfwSetInputMode(
            _p->_glfw_window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        _p->_has_grab = false;
    }

    activate();

    get_events().render(render_event(window_event::type::Render, this));

    glfwSwapBuffers(_p->_glfw_window_handle);
    glfwPollEvents();
}

void window::set_as_input_source(bool flag) { _p->_is_input_source = flag; }

bool window::get_is_input_source() const { return _p->_is_input_source; }

void window::set_can_grab(bool flag)
{
    _p->_can_grab = flag;
    if (!flag)
    {
        grab_mouse(flag);
    }
}

bool window::get_can_grab() const { return _p->_can_grab; }

void window::grab_mouse(bool flag)
{
    glfwSetInputMode(_p->_glfw_window_handle,
                     GLFW_CURSOR,
                     flag ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    _p->_has_grab = flag;
}

bool window::get_has_grab() const { return _p->_has_grab; }

void window::add_viewport(std::shared_ptr<viewport> vp)
{
    _p->_viewports.push_back(vp);
}

void window::remove_viewport(std::shared_ptr<viewport> vp)
{
    std::erase(_p->_viewports, vp);
}

std::vector<std::shared_ptr<viewport>> window::get_viewports()
{
    return _p->_viewports;
}

window_events& window::get_events() const { return _p->_events; }

void window::setup_mouse_callbacks()
{
    setup_mouse_enter_callback();
    setup_mouse_move_callback();
    setup_mouse_button_callback();
    setup_mouse_wheel_callback();
}

void window::setup_mouse_enter_callback()
{
    glfwSetCursorEnterCallback(_p->_glfw_window_handle,
                               [](GLFWwindow* wnd, int entered)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        window_events& events = _this->get_events();
        if (entered == 0)
        {
            events.leave(window_event(window_event::type::Leave, _this));
            return;
        }

        _this->set_as_input_source(true);
        glm::dvec2 pos;
        glfwGetCursorPos(wnd, &pos.x, &pos.y);
        _this->_p->_mouse_state._position = pos;
        events.enter(enter_event(window_event::type::Enter,
                                 _this->_p->_mouse_state._position,
                                 _this->_p->_mouse_state._position,
                                 _this->_p->_mouse_state._position,
                                 0,
                                 _this->_p->_mouse_state._buttons,
                                 _this->_p->_mouse_state._mods,
                                 _this));
    });
}

void window::setup_mouse_move_callback()
{
    glfwSetCursorPosCallback(
        _p->_glfw_window_handle,
        [](GLFWwindow* wnd, double x_position, double y_position)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        window_events& events = _this->get_events();
        _this->_p->_mouse_state._position = { x_position, y_position };

        if (_this->_p->_mouse_state._buttons)
        {
            _this->check_for_drag();
        }

        events.mouse_move(mouse_event(window_event::type::MouseMove,
                                      _this->_p->_mouse_state._position,
                                      _this->_p->_mouse_state._position,
                                      _this->_p->_mouse_state._position,
                                      0,
                                      _this->_p->_mouse_state._buttons,
                                      _this->_p->_mouse_state._mods,
                                      _this));

        if (_this->get_is_input_source())
        {
            input_system::set_mouse_position(_this->_p->_mouse_state._position);
        }
    });
}

void window::setup_mouse_button_callback()
{
    glfwSetMouseButtonCallback(
        _p->_glfw_window_handle,
        [](GLFWwindow* wnd, int button, int action, int mods)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        window_events& events = _this->get_events();
        if (action == GLFW_PRESS)
        {
            _this->_p->_mouse_state._press_started_position =
                _this->_p->_mouse_state._position;
            _this->_p->_mouse_state._buttons |= (1 << button);
            events.mouse_press(mouse_event(window_event::type::MouseButtonPress,
                                           _this->_p->_mouse_state._position,
                                           _this->_p->_mouse_state._position,
                                           _this->_p->_mouse_state._position,
                                           button,
                                           _this->_p->_mouse_state._buttons,
                                           _this->_p->_mouse_state._mods,
                                           _this));

            if (_this->get_can_grab())
            {
                _this->grab_mouse();
            }
        }
        else if (_this->_p->_mouse_state._buttons & (1 << button))
        {
            _this->_p->_mouse_state._buttons ^= (1 << button);

            events.mouse_release(
                mouse_event(window_event::type::MouseButtonRelease,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            button,
                            _this->_p->_mouse_state._buttons,
                            _this->_p->_mouse_state._mods,
                            _this));

            if (_this->_p->_mouse_state._is_drag)
            {
                _this->_p->_mouse_state._is_drag = false;
                // drag stopped
                return;
            }

            auto double_dur = std::chrono::steady_clock::now() -
                              _this->_p->_mouse_state._last_click_time;
            auto triple_dur = std::chrono::steady_clock::now() -
                              _this->_p->_mouse_state._last_double_click_time;

            int clicks = 0;

            if (!_this->_p->_mouse_state._is_drag)
            {
                clicks = 1;
                _this->_p->_mouse_state._last_click_time =
                    std::chrono::steady_clock::now();
            }

            if (double_dur < MAX_CLICK_REACTION_TIME)
            {
                clicks = 2;
                _this->_p->_mouse_state._last_double_click_time =
                    std::chrono::steady_clock::now();
            }

            if (triple_dur < MAX_CLICK_REACTION_TIME)
            {
                clicks = 3;
            }

            event<void(mouse_event)>* f = nullptr;

            window_event::type event_type =
                window_event::type::MouseButtonClick;

            switch (clicks)
            {
            case 1:
            {
                event_type = window_event::type::MouseButtonClick;
                f = &events.mouse_click;
                break;
            }
            case 2:
            {
                event_type = window_event::type::MouseButtonDoubleClick;
                f = &events.mouse_double_click;
                break;
            }
            case 3:
            {
                event_type = window_event::type::MouseButtonTripleClick;
                f = &events.mouse_triple_click;
                break;
            }
            default:
            {
                break;
            }
            }

            (*f)(mouse_event(event_type,
                             _this->_p->_mouse_state._position,
                             _this->_p->_mouse_state._position,
                             _this->_p->_mouse_state._position,
                             button,
                             _this->_p->_mouse_state._buttons,
                             _this->_p->_mouse_state._mods,
                             _this));

            if (_this->get_is_input_source())
            {
                input_system::set_mouse_button(
                    input_system::mouse_button::MouseButton0,
                    (_this->_p->_mouse_state._buttons ^= (1 << 0))
                        ? input_system::button_state::Press
                        : input_system::button_state::Release);
            }
        }
    });
}

void window::setup_mouse_wheel_callback()
{
    glfwSetScrollCallback(_p->_glfw_window_handle,
                          [](GLFWwindow* wnd, double x_offset, double y_offset)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        window_events& events = _this->get_events();

        events.mouse_scroll(wheel_event(
            window_event::type::MouseWheel,
            _this->_p->_mouse_state._position,
            _this->_p->_mouse_state._position,
            _this->_p->_mouse_state._position,
            0,
            _this->_p->_mouse_state._buttons,
            _this->_p->_mouse_state._mods,
            { static_cast<float>(x_offset), static_cast<float>(y_offset) },
            false,
            _this));
    });
}

void window::check_for_drag()
{
    double distance = glm::distance(_p->_mouse_state._position,
                                    _p->_mouse_state._press_started_position);
    if (distance > DRAG_START_DISTANCE)
    {
        _p->_mouse_state._is_drag = true;
        // drag started
    }
}

void window::configure_input_system()
{
    setup_mouse_callbacks();

    glfwSetKeyCallback(
        _p->_glfw_window_handle,
        [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));

        window_event::type type = window_event::type::KeyRelease;
        event<void(key_event)>* f = &_this->_p->_events.key_release;
        bool is_repeat = false;

        _this->_p->_mouse_state._mods =
            glfw_keyboard_modifiers_to_mouse_event_modifiers(mods);

        if (action == GLFW_PRESS)
        {
            type = window_event::type::KeyPress;
            f = &_this->_p->_events.key_press;
        }
        else if (action == GLFW_REPEAT)
        {
            type = window_event::type::KeyRepeat;
            f = &_this->_p->_events.key_repeat;
            is_repeat = true;
        }
        (*f)(key_event(
            type, scancode, _this->_p->_mouse_state._mods, is_repeat, _this));

        if (_this->get_is_input_source())
        {
            input_system::set_key_down(key, action != GLFW_RELEASE);
        }
    });
}

} // namespace experimental
