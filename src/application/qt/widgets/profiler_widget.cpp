#include <QLayout>
#include <QWindow>

#include <core/window.hpp>
#include <tools/profiler/profiler_renderer.hpp>

#include "application/qt/widgets/profiler_widget.hpp"

struct ProfilerWidget::impl
{
    QWidget* _window_widget = nullptr;
    std::shared_ptr<core::window> _window = nullptr;
    std::unique_ptr<profiler_renderer> _profilerRenderer = nullptr;
};

ProfilerWidget::ProfilerWidget(QWidget* parent)
    : QWidget(parent)
    , _p(std::make_unique<impl>())
{
    setLayout(new QVBoxLayout());
}

ProfilerWidget* ProfilerWidget::create(QWidget* parent)
{
    ProfilerWidget* widget = new ProfilerWidget(parent);

    widget->_p->_window = std::make_shared<core::window>();
    widget->_p->_window->on_user_initialize += [ widget ](auto window)
    {
        window->get_events()->render += [ widget ](const core::render_event& e)
        {
            auto wnd = widget->_p->_window;
            auto size = wnd->get_size();
            auto zoom = glm::vec2(.3f, .0001f);
            auto scroll = glm::vec2(0.0f, 0.0f);
            widget->_p->_profilerRenderer->render(size, zoom, scroll);
        };
    };

    widget->_p->_window->init();
    auto wid = widget->_p->_window->get_native_handle();

    QWindow* w = QWindow::fromWinId(reinterpret_cast<WId>(wid));
    widget->_p->_window_widget = QWidget::createWindowContainer(w, parent);

    widget->layout()->addWidget(widget->_p->_window_widget);

    return widget;
}
