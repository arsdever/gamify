#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "core/input_system.hpp"

#include "common/logging.hpp"
#include "common/utils.hpp"
#include "core/inputs/binding.hpp"
#include "core/window.hpp"

namespace core
{
namespace
{
inline logger log() { return get_logger("input_system"); }
} // namespace

void input_system::update_device_list()
{
    for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
    {
        if (glfwJoystickIsGamepad(i))
        {
            std::string uid = glfwGetJoystickGUID(i);
            std::string name = glfwGetJoystickName(i);
            log()->info("Joystick {} present: {}: {}", i, uid, name);
            _gamepads.push_back({ i });
        }
    }
}

void input_system::set_input_source(std::shared_ptr<window> input_source)
{
    if (input_source == _input_source.lock())
        return;

    _input_source = input_source;
    input_source->activate();
    auto handle = glfwGetCurrentContext();

    glfwSetKeyCallback(
        handle,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_UNKNOWN)
            return;

        input_system::set_modifiers(static_cast<modifiers>(mods));

        if (action == GLFW_PRESS)
            input_system::set_key_down(key, true);
        else if (action == GLFW_RELEASE)
            input_system::set_key_down(key, false);
    });

    glfwSetCursorPosCallback(handle,
                             [](GLFWwindow* window, double x, double y)
    { input_system::set_mouse_position({ x, y }); });

    glfwSetMouseButtonCallback(
        handle,
        [](GLFWwindow* window, int button, int action, int mods)
    {
        input_system::set_modifiers(static_cast<modifiers>(mods));

        auto evt =
            action == GLFW_PRESS ? button_state::press : button_state::release;
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            input_system::set_mouse_button(mouse_button::left, evt);
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            input_system::set_mouse_button(mouse_button::right, evt);
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            input_system::set_mouse_button(mouse_button::middle, evt);
    });

    // glfwSetScrollCallback(handle,
    //                       [](GLFWwindow* window, double xoffset, double
    //                       yoffset)
    // {
    //     input_system::set_mouse_delta({ xoffset, yoffset });
    // });

    glfwSetJoystickCallback([](int jid, int event)
    {
        if (event == GLFW_CONNECTED)
            input_system::update_device_list();
    });

    update_device_list();
}

std::shared_ptr<window> input_system::get_input_source()
{
    return _input_source.lock();
}

void input_system::set_mouse_position(glm::ivec2 pos)
{
    _mouse_delta = pos - _mouse_position;
    _mouse_position = pos;
}

glm::ivec2 input_system::get_mouse_position() { return _mouse_position; }

glm::ivec2 input_system::get_mouse_delta() { return _mouse_delta; }

input_system::button_state
input_system::get_mouse_button(mouse_button button_code)
{
    auto it = _mouse_buttons_state.find(button_code);
    if (it == _mouse_buttons_state.end())
        return button_state::unspecified;

    return it->second;
}

void input_system::set_mouse_button(mouse_button button_code,
                                    button_state state)
{
    _mouse_buttons_state[ button_code ] = state;
}

bool input_system::is_key_down(int keycode)
{
    return _pressed_keys.contains(keycode);
}

void input_system::set_key_down(int keycode, bool state)
{
    if (state)
    {
        _pressed_keys.emplace(keycode);
        on_keypress(keycode);
    }
    else
    {
        _pressed_keys.erase(keycode);
    }
}

void input_system::set_modifiers(modifiers modifiers)
{
    _modifiers = modifiers;
}

input_system::modifiers input_system::get_modifiers() { return _modifiers; }

std::shared_ptr<binding> input_system::declare_input(std::string_view name)
{
    auto it = _mapping.find(name);
    if (it != _mapping.end())
        return it->second;

    auto b = std::make_shared<binding>();
    _mapping.emplace(name, b);
    return b;
}

std::shared_ptr<binding> input_system::get_input(std::string_view name)
{
    auto it = _mapping.find(name);
    if (it == _mapping.end())
        return nullptr;

    return it->second;
}

std::shared_ptr<binding> input_system::bind_input(std::shared_ptr<binding> bond,
                                                  std::string_view path)
{
    size_t pos = path.find_first_of(".");
    std::string_view device = path.substr(0, pos);
    if (device == "mouse") { }
    else if (device == "keyboard")
    {
        std::string_view device_input = path.substr(pos + 1);
        if (device_input == "spacebar")
        {
            bond->set_updater([]
            {
                return 2 * static_cast<float>(is_key_down(GLFW_KEY_SPACE)) - 1;
            });
        }
    }
    else if (device == "gamepad")
    {
        std::string_view device_input = path.substr(pos + 1);
        if (device_input == "left_joystick")
        {
            bond->set_updater([]() -> glm::vec2
            {
                if (_gamepads.empty())
                    return {};

                return _gamepads[ 0 ].get_left_axis();
            });
        }
        else if (device_input == "right_joystick")
        {
            bond->set_updater([]() -> glm::vec2
            {
                if (_gamepads.empty())
                    return {};

                return _gamepads[ 0 ].get_right_axis();
            });
        }
        else if (device_input == "left_trigger")
        {
            bond->set_updater([]() -> float
            {
                if (_gamepads.empty())
                    return {};

                return _gamepads[ 0 ].get_left_trigger();
            });
        }
        else if (device_input == "right_trigger")
        {
            bond->set_updater([]() -> float
            {
                if (_gamepads.empty())
                    return {};

                return _gamepads[ 0 ].get_right_trigger();
            });
        }
    }

    return bond;
}

std::shared_ptr<binding> input_system::bind_input(std::string_view name,
                                                  std::string_view path)
{
    return bind_input(declare_input(name), path);
}

std::unordered_set<int> input_system::_pressed_keys;
std::unordered_map<input_system::mouse_button, input_system::button_state>
    input_system::_mouse_buttons_state;
event<void(int)> input_system::on_keypress;
glm::ivec2 input_system::_mouse_position;
glm::ivec2 input_system::_mouse_delta;
input_system::modifiers input_system::_modifiers;
std::vector<inputs::gamepad> input_system::_gamepads;
std::unordered_map<std::string,
                   std::shared_ptr<binding>,
                   string_hash,
                   std::equal_to<>>
    input_system::_mapping;
std::weak_ptr<window> input_system::_input_source;
} // namespace core
