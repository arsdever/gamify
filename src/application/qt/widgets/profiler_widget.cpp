#include <QLayout>
#include <QToolBar>
#include <QToolTip>
#include <QWheelEvent>
#include <QWindow>

#include <GLFW/glfw3.h>
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
    std::optional<QPointF> _pan_start;

    std::optional<std::variant<const prof::frame*, const prof::data_sample*>>
        _tooltip_target;

    QToolBar* _toolbar = nullptr;

    bool _frame_mode = false;
};

ProfilerWidget::ProfilerWidget(QWidget* parent)
    : QWidget(parent)
    , _p(std::make_unique<impl>())
{
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
}

ProfilerWidget* ProfilerWidget::create(QWidget* parent)
{
    ProfilerWidget* widget = new ProfilerWidget(parent);

    widget->_p->_profiler = std::make_shared<profiler>();
    widget->_p->_toolbar =
        new QToolBar("Profiler Toolbar", widget->_p->_window_widget);

    Q_INIT_RESOURCE(resources);
    auto toolbar = widget->_p->_toolbar;
    toolbar->addAction(QIcon(":/res/icons/zoom_to_fit.png"),
                       "Reset zoom",
                       [ widget ]() { widget->setZoom({ 1, 1 }); });
    toolbar->addAction(QIcon(":/res/icons/overview.png"),
                       "Overall view",
                       [ widget ]() { widget->reset(); });
    QAction* startAction = toolbar->addAction(
        QIcon(":/res/icons/play.png"),
        "Resume profiling");
    QAction* stopAction = toolbar->addAction(
        QIcon(":/res/icons/stop.png"),
        "Pause profiling");
    stopAction->setVisible(false);

    connect(startAction,
            &QAction::triggered,
            [ widget, startAction, stopAction ]()
    {
        prof::start();
        startAction->setVisible(false);
        stopAction->setVisible(true);
    });

    connect(stopAction,
            &QAction::triggered,
            [ widget, startAction, stopAction ]()
    {
        prof::stop();
        startAction->setVisible(true);
        stopAction->setVisible(false);
    });
    widget->layout()->addWidget(toolbar);

    widget->_p->_profiler->init();
    auto wid = widget->_p->_profiler->get_native_handle();

    auto profiler = widget->_p->_profiler;
    profiler->get_events()->mouse_click +=
        [ widget, p = std::weak_ptr(profiler) ](auto me)
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
                widget->_p->_frame_mode = true;
            }
        }
        emit widget->frameSelected();
    };

    profiler->get_events()->mouse_press +=
        [ widget, p = std::weak_ptr(profiler) ](auto me)
    {
        if (me.get_button() == 2)
        {
            widget->startPanning(QPointF(me.get_local_position().x, 0));
        }
    };

    profiler->get_events()->mouse_release +=
        [ widget, p = std::weak_ptr(profiler) ](auto me)
    {
        if (me.get_button() == 2)
        {
            widget->stopPanning(QPointF(me.get_local_position().x, 0));
        }
    };

    profiler->get_events()->mouse_move +=
        [ widget, p = std::weak_ptr(profiler) ](auto me)
    {
        if (widget->_p->_pan_start.has_value())
        {
            if (me.get_buttons() & (1 << 2))
            {
                widget->stopPanning(QPointF(me.get_local_position().x, 0));
                widget->startPanning(QPointF(me.get_local_position().x, 0));
            }
            else
            {
                widget->stopPanning(QPointF(me.get_local_position().x, 0));
            }
        }

        if (auto profiler = p.lock())
        {
            auto elem = profiler->get_at(me.get_local_position());
            if (widget->_p->_tooltip_target == elem)
            {
                return;
            }

            widget->_p->_tooltip_target = elem;

            if (!elem.has_value())
            {
                QToolTip::hideText();
                return;
            }

            typename profiler::element_type element = elem.value();
            if (widget->_p->_frame_mode &&
                std::holds_alternative<const prof::data_sample*>(element))
            {
                auto data = std::get<const prof::data_sample*>(element);

                QToolTip::showText(
                    QCursor::pos(),
                    QString::fromStdString(
                        std::format("Sample: {}\nDuration: {}\nStack depth: {}",
                                    data->name(),
                                    format_scaled(data->diff()),
                                    data->depth())),
                    widget,
                    {});
            }
            else if (std::holds_alternative<const prof::frame*>(element))
            {
                auto fp = std::get<const prof::frame*>(element);
                QToolTip::showText(QCursor::pos(),
                                   QString::fromStdString(std::format(
                                       "Frame: #{}\nDuration: {}",
                                       fp->get_id(),
                                       format_scaled(std::chrono::duration_cast<
                                                     std::chrono::microseconds>(
                                           fp->end() - fp->start())))),
                                   widget,
                                   {});
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
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 0)
        {
            setZoom(zoom() * 1.2);
        }
        else
        {
            setZoom(zoom() * 0.8);
        }
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        setScroll(scroll() +
                  glm::dvec2 { event->angleDelta().y() / 72.0, 0.0 });
    }
    else
    {
        setScroll(scroll() +
                  glm::dvec2 { 0.0, event->angleDelta().y() / 72.0 });
    }
    event->accept();
}

void ProfilerWidget::startPanning(QPointF position)
{
    _p->_pan_start = position;
}

void ProfilerWidget::stopPanning(QPointF position)
{
    if (_p->_pan_start.has_value())
    {
        auto delta = position - _p->_pan_start.value();
        setScroll(scroll() + glm::dvec2 { -delta.x(), delta.y() });
        _p->_pan_start.reset();
    }
}

glm::dvec2 ProfilerWidget::zoom() const { return _p->_profiler->zoom(); }

glm::dvec2 ProfilerWidget::scroll() const { return _p->_profiler->scroll(); }

QSize ProfilerWidget::sizeHint() const
{
    if (_p->_frame_mode)
    {
        return size();
    }

    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto duration = 0;
    auto* lf = prof::longest_frame(ss.str());
    if (lf)
    {
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                       lf->end() - lf->start())
                       .count();
    }
    glm::vec2 size =
        glm::vec2(prof::available_frames_count(ss.str()) - 1, duration) *
        _p->_profiler->get_sample_size();

    return QSize(size.x, size.y);
}

void ProfilerWidget::reset()
{
    _p->_profiler->unset_frame();
    _p->_frame_mode = false;
}

void ProfilerWidget::setZoom(glm::dvec2 z)
{
    _p->_profiler->set_zoom(std::move(z));
}

void ProfilerWidget::setScroll(glm::dvec2 s)
{
    _p->_profiler->scroll_to(std::move(s));
    scrollChanged();
}
