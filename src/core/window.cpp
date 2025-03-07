/* clang-format off */
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
/* clang-format on */

#include <glm/ext.hpp>
#include <prof/profiler.hpp>

#include "core/window.hpp"

#include "common/logging.hpp"
#include "core//input_system.hpp"
#include "core/settings.hpp"
#include "core/timer.hpp"
#include "core/window_events.hpp"
#include "graphics/graphics.hpp"

namespace
{
logger log() { return get_logger("window"); }

core::mouse_event::modifiers
glfw_keyboard_modifiers_to_mouse_event_modifiers(int mods)
{
    core::mouse_event::modifiers modifiers;
    modifiers = static_cast<core::mouse_event::modifiers>(mods);
    return modifiers;
}

static constexpr std::chrono::milliseconds MAX_CLICK_REACTION_TIME { 300 };
static constexpr double DRAG_START_DISTANCE { 5.0 };

} // namespace

namespace core
{

struct window::window_private_data
{
    GLFWwindow* _glfw_window_handle { nullptr };
    glm::uvec2 _size { 800, 600 };
    glm::uvec2 _position { 200, 200 };
    std::shared_ptr<window_events> _events { nullptr };
    std::vector<std::shared_ptr<viewport>> _viewports;
    std::string _title { "Window" };
    bool _is_main_window { false };
    bool _can_grab { false };
    bool _has_grab { false };

    struct
    {
        int _buttons;
        mouse_event::modifiers _mods;
        glm::vec2 _position;
        glm::vec2 _press_started_position;
        core::timer _double_click_timer;
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
    if (!_main_window)
    {
        _main_window = shared_from_this();
        _p->_is_main_window = true;
    }

    if (!core::settings.contains("antialiasing"))
    {
        core::settings[ "antialiasing" ] = 4;
    }

    glfwWindowHint(GLFW_SAMPLES,
                   core::settings[ "antialiasing" ].get<unsigned>());
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _p->_glfw_window_handle = glfwCreateWindow(
        static_cast<int>(get_width()),
        static_cast<int>(get_height()),
        title.c_str(),
        nullptr,
        _p->_is_main_window ? nullptr : _main_window->_p->_glfw_window_handle);
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
        if (_this->_p->_size == glm::uvec2 { w, h })
        {
            return;
        }

        glm::uvec2 old_size = _this->_p->_size;
        _this->_p->_size = { w, h };
        _this->get_events()->resize(
            resize_event(old_size, _this->_p->_size, _this));
    });

    glfwSetWindowPosCallback(_p->_glfw_window_handle,
                             [](GLFWwindow* wnd, int xpos, int ypos)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));
        glm::uvec2 old_pos = _this->_p->_position;
        _this->_p->_position = { xpos, ypos };
        _this->get_events()->move(
            move_event(old_pos, _this->_p->_position, _this));
    });

    glfwSetInputMode(_p->_glfw_window_handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

    activate();

    if (!graphics::initialize(glfwGetProcAddress))
    {
        return;
    }

    configure_input_system();
    on_user_initialize(shared_from_this());
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
    if (_p->_size == glm::uvec2 { width, height })
    {
        return;
    }

    _p->_size = { width, height };
    if (_p->_glfw_window_handle == nullptr)
    {
        return;
    }
    glfwSetWindowSize(_p->_glfw_window_handle,
                      static_cast<int>(width),
                      static_cast<int>(height));
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
        glfwDestroyWindow(_p->_glfw_window_handle);
        _p->_glfw_window_handle = nullptr;
        get_events()->close(close_event(this));
        return;
    }

    if (input_system::is_key_down(345) && input_system::is_key_down(346))
    {
        glfwSetInputMode(
            _p->_glfw_window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        _p->_has_grab = false;
    }

    activate();

    get_events()->render(render_event(window_event::type::Render, this));

    if (get_is_input_source())
    {
        // reset the mouse delta
        input_system::set_mouse_position(input_system::get_mouse_position());
    }
    glfwSwapBuffers(_p->_glfw_window_handle);
    glfwPollEvents();
}

void window::set_as_input_source(bool flag)
{
    input_system::set_input_source(shared_from_this());
}

bool window::get_is_input_source() const
{
    return input_system::get_input_source() == shared_from_this();
}

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

std::shared_ptr<window_events> window::get_events() const
{
    return _p->_events;
}

std::shared_ptr<window> window::get_main_window() { return _main_window; }

void* window::get_native_handle() const
{
    return glfwGetWin32Window(_p->_glfw_window_handle);
}

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
        std::shared_ptr<window_events> events = _this->get_events();
        if (entered == 0)
        {
            events->leave(window_event(window_event::type::Leave, _this));
            return;
        }

        glm::dvec2 pos;
        glfwGetCursorPos(wnd, &pos.x, &pos.y);
        _this->_p->_mouse_state._position = pos;
        events->enter(enter_event(window_event::type::Enter,
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
        std::shared_ptr<window_events> events = _this->get_events();
        _this->_p->_mouse_state._position = { x_position, y_position };

        if (_this->_p->_mouse_state._buttons)
        {
            _this->check_for_drag();
        }

        events->mouse_move(mouse_event(window_event::type::MouseMove,
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
        _this->_p->_mouse_state._buttons ^= (1 << button);
        std::shared_ptr<window_events> events = _this->get_events();
        if (action == GLFW_PRESS)
        {
            _this->_p->_mouse_state._press_started_position =
                _this->_p->_mouse_state._position;
            events->mouse_press(
                mouse_event(window_event::type::MouseButtonPress,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            button,
                            _this->_p->_mouse_state._buttons,
                            _this->_p->_mouse_state._mods,
                            _this));

            auto& dct = _this->_p->_mouse_state._double_click_timer;
            if (dct.is_running())
            {
                events->mouse_double_click(
                    mouse_event(window_event::type::MouseButtonDoubleClick,
                                _this->_p->_mouse_state._position,
                                _this->_p->_mouse_state._position,
                                _this->_p->_mouse_state._position,
                                button,
                                _this->_p->_mouse_state._buttons,
                                _this->_p->_mouse_state._mods,
                                _this));
            }

            if (_this->get_can_grab())
            {
                _this->grab_mouse();
            }
        }
        else if (!(_this->_p->_mouse_state._buttons & (1 << button)))
        {
            events->mouse_release(
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

            events->mouse_click(
                mouse_event(window_event::type::MouseButtonClick,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            _this->_p->_mouse_state._position,
                            button,
                            _this->_p->_mouse_state._buttons,
                            _this->_p->_mouse_state._mods,
                            _this));
            auto& dct = _this->_p->_mouse_state._double_click_timer;

            if (!dct.is_running())
            {
                dct.wait();
                dct.reset();
                dct.start(MAX_CLICK_REACTION_TIME);
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
        std::shared_ptr<window_events> events = _this->get_events();

        events->mouse_scroll(wheel_event(
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
    _p->_events = std::make_shared<window_events>();
    setup_mouse_callbacks();

    glfwSetKeyCallback(
        _p->_glfw_window_handle,
        [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
    {
        auto _this = static_cast<window*>(glfwGetWindowUserPointer(wnd));

        window_event::type type = window_event::type::KeyRelease;
        event<void(key_event)>* f = &_this->_p->_events->key_release;
        bool is_repeat = false;

        _this->_p->_mouse_state._mods =
            glfw_keyboard_modifiers_to_mouse_event_modifiers(mods);

        if (action == GLFW_PRESS)
        {
            type = window_event::type::KeyPress;
            f = &_this->_p->_events->key_press;
        }
        else if (action == GLFW_REPEAT)
        {
            type = window_event::type::KeyRepeat;
            f = &_this->_p->_events->key_repeat;
            is_repeat = true;
        }
        (*f)(key_event(type,
                       scancode,
                       key,
                       _this->_p->_mouse_state._mods,
                       is_repeat,
                       _this));

        if (_this->get_is_input_source())
        {
            input_system::set_key_down(key, action != GLFW_RELEASE);
            input_system::set_modifiers(input_system::modifiers(mods));
        }
    });
}

std::shared_ptr<window> window::_main_window = nullptr;

} // namespace core
