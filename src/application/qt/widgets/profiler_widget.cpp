#include <QLayout>
#include <QWheelEvent>
#include <QWindow>

#include <core/window.hpp>
#include <tools/profiler/profiler_renderer.hpp>

#include "application/qt/widgets/profiler_widget.hpp"

struct ProfilerWidget::impl
{
    QWidget* _window_widget = nullptr;
    std::shared_ptr<core::window> _window = nullptr;
    std::unique_ptr<profiler_renderer> _profilerRenderer = nullptr;

    glm::dvec2 _zoom { .3f, .0001f };
    glm::dvec2 _scroll { 0.0f, 0.0f };
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
            widget->_p->_profilerRenderer->render(
                size, widget->_p->_zoom, widget->_p->_scroll);
        };
    };

    widget->_p->_window->init();
    auto wid = widget->_p->_window->get_native_handle();

    QWindow* w = QWindow::fromWinId(reinterpret_cast<WId>(wid));
    widget->_p->_window_widget = QWidget::createWindowContainer(w, parent);

    widget->layout()->addWidget(widget->_p->_window_widget);

    return widget;
}

void ProfilerWidget::wheelEvent(QWheelEvent* event)
{
    // Ctrl + Wheel = ZoomY
    // Ctrl + Shift + Wheel = ZoomX
    // Wheel = ScrollY
    // Shift + Wheel = ScrollX
    if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
    {
        _p->_zoom.x *= event->angleDelta().y() / 72.0f;
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        _p->_zoom.y *= event->angleDelta().y() / 72.0f;
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        _p->_scroll.x += event->angleDelta().x() / 72.0f;
    }
    else
    {
        _p->_scroll.y += event->angleDelta().y() / 72.0f;
    }
    // _p->_scroll *= glm::vec2(event->angleDelta().x(),
    // event->angleDelta().y()) / 72.0f;
    event->accept();
}

glm::dvec2 ProfilerWidget::zoom() const { return _p->_zoom; }

glm::dvec2 ProfilerWidget::scroll() const { return _p->_scroll; }

void ProfilerWidget::setZoom(glm::dvec2 z) { _p->_zoom = std::move(z); }

void ProfilerWidget::setScroll(glm::dvec2 s) { _p->_scroll = std::move(s); }
