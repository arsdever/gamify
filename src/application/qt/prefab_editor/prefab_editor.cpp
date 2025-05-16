#include <QApplication>
#include <QWindow>

#include <common/main_thread_dispatcher.hpp>
#include <core/input_system.hpp>
#include <core/window.hpp>
#include <prof/profiler.hpp>
#include <scripting/backend.hpp>

#include "application/qt/widgets/profiler_window.hpp"
#include "graphics/graphics.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <application/qt/widgets/inspector_widget.hpp>

#include "editor_main_window.hpp"
#include "game_context.hpp"

int main(int argc, char** argv)
{
    int exit_code;

    glfwInit();

    QApplication app(argc, argv);
    auto qt_window = new EditorMainWindow();
    qt_window->setWindowTitle("Prefab Editor");

    common::main_thread_dispatcher::initialize();

    auto gl_window = std::make_shared<core::window>();

    gl_window->on_user_initialize += [ gl_window ](auto)
    {
        game_context::initialize();
        core::input_system::set_input_source(gl_window);
        core::input_system::update_device_list();
        scripting::backend::initialize();
        game_context::load_assets();
    };
    gl_window->init();
    gl_window->get_events()->render += [](auto) { game_context::render(); };
    gl_window->get_events()->resize += [](auto re)
    {
        auto new_size = re.get_new_size();
        game_context::set_viewport_size(new_size);
    };

    QWindow* qt_gl_window_handle =
        QWindow::fromWinId((WId)(gl_window->get_native_handle()));
    QWidget* gl_widget = QWidget::createWindowContainer(qt_gl_window_handle);

    qt_window->setCentralWidget(gl_widget);
    qt_window->resize(1280, 720);
    qt_window->show();

    int result = 0;

    game_context::init();

    while (qt_window->isVisible())
    {
        auto p = prof::profile_frame("Main Loop");

        // Process events in the Qt main loop
        {
            auto qtp = prof::profile("Qt Main Loop");
            app.processEvents();
        }

        // Update the game context
        game_context::update();
        common::main_thread_dispatcher::run_all();

        // Process events in the GLFW main loop
        core::window::visit_windows([](auto w) { w->update(); });
    }

    return result;
}
