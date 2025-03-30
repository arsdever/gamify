#include <QApplication>
#include <QMainWindow>
#include <QWindow>

#include <common/main_thread_dispatcher.hpp>
#include <core/window.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

int main(int argc, char** argv)
{
    int exit_code;

    QApplication app(argc, argv);
    auto qt_window = new QMainWindow();
    qt_window->setWindowTitle("Prefab Editor");

    common::main_thread_dispatcher::initialize();

    glfwInit();

    auto gl_window = std::make_shared<core::window>();
    gl_window->init();

    QWindow* qt_gl_window_handle =
        QWindow::fromWinId((WId)(gl_window->get_native_handle()));
    QWidget* gl_widget = QWidget::createWindowContainer(qt_gl_window_handle);

    qt_window->setCentralWidget(gl_widget);
    qt_window->resize(1280, 720);
    qt_window->show();

    int result = 0;

    while (qt_window->isVisible())
    {
        // Process events in the Qt main loop
        app.processEvents();

        // Process events in the GLFW main loop
        gl_window->update();
    }

    return result;
}
