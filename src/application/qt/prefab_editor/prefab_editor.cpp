#include <QApplication>
#include <QWindow>

#include <common/main_thread_dispatcher.hpp>
#include <core/window.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

#include "editor_main_window.hpp"
#include "game_context.hpp"

int main(int argc, char** argv)
{
    int exit_code;

    QApplication app(argc, argv);
    auto qt_window = new EditorMainWindow();
    qt_window->setWindowTitle("Prefab Editor");

    common::main_thread_dispatcher::initialize();

    glfwInit();

    auto gl_window = std::make_shared<core::window>();

    gl_window->on_user_initialize += [](auto)
    {
        game_context::initialize();
        game_context::load_assets();
    };
    gl_window->init();

    gl_window->get_events()->render += [](auto) { game_context::render(); };

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
        // Process events in the Qt main loop
        app.processEvents();

        // Update the game context
        game_context::update();

        // Process events in the GLFW main loop
        gl_window->update();
    }

    return result;
}
