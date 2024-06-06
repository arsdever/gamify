/* clang-format off */
#include <glad/gl.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#include <functional>
#include <memory>
#include <vector>

#include <assert.h>

#include "experimental/window.hpp"

#include "gl_error_handler.hpp"
#include "logging.hpp"

namespace
{
logger log() { return get_logger("window example"); }
} // namespace

int main(int argc, char** argv)
{
    glfwInit();
    std::vector<std::shared_ptr<experimental::window>> windows;
    auto exp_window = std::make_shared<experimental::window>();

    exp_window->on_user_initialize += [](std::shared_ptr<experimental::window>)
    {
        // configure gl debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_error_handler, nullptr);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    };

    exp_window->set_title("Hello experimental window");
    exp_window->init();

    exp_window->set_can_grab(true);

    exp_window->get_events().close += [ &windows ](auto ce)
    {
        (void)ce;
        assert(ce.get_sender() != nullptr);
        log()->info("The window is closing");
        std::erase(windows, ce.get_sender()->shared_from_this());
    };

    exp_window->get_events().leave += [](auto ee)
    {
        (void)ee;
        assert(ee.get_sender() != nullptr);
        log()->info("Cursor left the window");
    };

    exp_window->get_events().enter += [](auto ee)
    {
        assert(ee.get_sender() != nullptr);
        log()->info("Cursor entered the window at: {}x{}",
                    ee.get_local_position().x,
                    ee.get_local_position().y);
    };

    exp_window->get_events().move += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Window moved from: {}x{} to: {}x{}",
                    me.get_old_position().x,
                    me.get_old_position().y,
                    me.get_new_position().x,
                    me.get_new_position().y);
    };

    exp_window->get_events().resize += [](auto re)
    {
        assert(re.get_sender() != nullptr);
        log()->info("Window resized from: {}x{} to: {}x{}",
                    re.get_old_size().x,
                    re.get_old_size().y,
                    re.get_new_size().x,
                    re.get_new_size().y);
    };

    exp_window->get_events().mouse_move += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse position: {}x{}",
                    me.get_local_position().x,
                    me.get_local_position().y);
    };

    exp_window->get_events().mouse_release += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse release: {}", me.get_button());
    };

    exp_window->get_events().mouse_press += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse press: {}", me.get_button());
    };

    exp_window->get_events().mouse_click += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse click: {}", me.get_button());
    };

    exp_window->get_events().mouse_double_click += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse double click: {}", me.get_button());
    };

    exp_window->get_events().mouse_triple_click += [](auto me)
    {
        assert(me.get_sender() != nullptr);
        log()->info("Mouse triple click: {}", me.get_button());
    };

    exp_window->get_events().mouse_scroll += [](auto we)
    {
        assert(we.get_sender() != nullptr);
        log()->info("Mouse scroll: {}x{}", we.get_delta().x, we.get_delta().y);
    };

    exp_window->get_events().key_press += [](auto ke)
    {
        assert(ke.get_sender() != nullptr);
        log()->info("Key pressed: {}", ke.get_scancode());
    };

    exp_window->get_events().key_repeat += [](auto ke)
    {
        assert(ke.get_sender() != nullptr);
        log()->info("Key repeated: {}", ke.get_scancode());
    };

    exp_window->get_events().key_release += [](auto ke)
    {
        assert(ke.get_sender() != nullptr);
        log()->info("Key released: {}", ke.get_scancode());
    };

    windows.push_back(exp_window);

    while (!windows.empty())
    {
        for (int i = 0; i < windows.size(); ++i)
        {
            auto wnd = windows[ i ];
            wnd->update();
        }
    }

    return 0;
}
