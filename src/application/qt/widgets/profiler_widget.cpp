#include <QLayout>
#include <QWheelEvent>
#include <QWindow>
#include <variant>

#include <common/logging.hpp>
#include <core/window.hpp>
#include <tools/profiler/profiler.hpp>

#include "application/qt/widgets/profiler_widget.hpp"

#include "prof/profiler.hpp"


namespace
{
logger log() { return get_logger("profiler"); }
} // namespace

struct ProfilerWidget::impl
{
    QWidget* _window_widget = nullptr;
    std::shared_ptr<profiler> _profiler = nullptr;

    std::optional<prof::frame> _frame;
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

    widget->_p->_profiler = std::make_shared<profiler>();
    widget->_p->_profiler->init();
    auto wid = widget->_p->_profiler->get_native_handle();

    auto profiler = widget->_p->_profiler;
    profiler->get_events()->mouse_click +=
        [ p = std::weak_ptr(profiler) ](auto me)
    {
        if (auto profiler = p.lock())
        {
            auto elem = profiler->get_at(me.get_local_position());

            if (!elem.has_value())
            {
                return;
            }

            typename profiler::element_type element = elem.value();
            if (std::holds_alternative<const prof::frame*>(element))
            {
                auto fp = std::get<const prof::frame*>(element);
                log()->info("Visualizing frame #{}", fp->get_id());
                profiler->set_frame(*fp);
            }
        }
    };

    QWindow* w = QWindow::fromWinId(reinterpret_cast<WId>(wid));
    widget->_p->_window_widget = QWidget::createWindowContainer(w, parent);

    widget->layout()->addWidget(widget->_p->_window_widget);

    return widget;
}

void ProfilerWidget::wheelEvent(QWheelEvent* event)
{
    auto profiler = _p->_profiler;
    // Ctrl + Wheel = ZoomY
    // Ctrl + Shift + Wheel = ZoomX
    // Wheel = ScrollY
    // Shift + Wheel = ScrollX
    if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
    {
        profiler->set_zoom(zoom() *
                           glm::dvec2 { event->angleDelta().y() / 72.0, 1.0 });
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        profiler->set_zoom(zoom() *
                           glm::dvec2 { 1.0, event->angleDelta().y() / 72.0 });
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        profiler->scroll_to(scroll() +
                            glm::dvec2 { event->angleDelta().x() / 72.0, 1.0 });
    }
    else
    {
        profiler->scroll_to(scroll() +
                            glm::dvec2 { 0.0, event->angleDelta().y() / 72.0 });
    }
    // _p->_scroll *= glm::vec2(event->angleDelta().x(),
    // event->angleDelta().y()) / 72.0f;
    event->accept();
}

glm::dvec2 ProfilerWidget::zoom() const { return _p->_profiler->zoom(); }

glm::dvec2 ProfilerWidget::scroll() const { return _p->_profiler->scroll(); }

void ProfilerWidget::reset()
{
    _p->_profiler->unset_frame();
}

void ProfilerWidget::setZoom(glm::dvec2 z)
{
    _p->_profiler->set_zoom(std::move(z));
}

void ProfilerWidget::setScroll(glm::dvec2 s)
{
    _p->_profiler->scroll_to(std::move(s));
}
