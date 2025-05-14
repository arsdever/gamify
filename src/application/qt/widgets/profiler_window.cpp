#include <QGridLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QToolButton>

#include <application/qt/widgets/profiler_widget.hpp>
#include <common/logging.hpp>
#include <prof/profiler.hpp>
#include <qgridlayout.h>
#include <qtimer.h>

#include "profiler_window.hpp"

#include "application/qt/widgets/multi_spinbox.hpp"

namespace
{
logger log() { return get_logger("profiler"); }
} // namespace

struct ProfilerWindow::impl
{
    ProfilerWindow* _this { nullptr };
    ProfilerWidget* _widget { nullptr };

    QScrollBar* _scroll_barh { nullptr };
    QScrollBar* _scroll_barv { nullptr };

    void setup();
    void setup_scroll_bars();
    void setup_profiler_widget();
    void update_scroll_bars();
};

void ProfilerWindow::impl::setup()
{
    setup_profiler_widget();
    setup_scroll_bars();
}

void ProfilerWindow::impl::setup_profiler_widget()
{
    _widget = ProfilerWidget::create();
    _this->connect(_widget,
                   &ProfilerWidget::frameSelected,
                   [ this ] { update_scroll_bars(); });
    _this->connect(_widget,
                   &ProfilerWidget::scrollChanged,
                   [ this ]
    {
        auto b = _widget->blockSignals(true);
        update_scroll_bars();
        _widget->blockSignals(b);
    });
}

void ProfilerWindow::impl::setup_scroll_bars()
{
    _scroll_barh = new QScrollBar(_this);
    _scroll_barh->setOrientation(Qt::Orientation::Horizontal);
    _scroll_barh->setFixedHeight(15);
    _scroll_barv = new QScrollBar(_this);
    _scroll_barv->setOrientation(Qt::Orientation::Vertical);
    _scroll_barv->setFixedWidth(15);

    _this->connect(_scroll_barh,
                   &QScrollBar::valueChanged,
                   [ this ](int value)
    {
        _widget->setScroll({ static_cast<double>(value), _widget->scroll().y });
    });

    _this->connect(_scroll_barv,
                   &QScrollBar::valueChanged,
                   [ this ](int value)
    {
        _widget->setScroll({ _widget->scroll().x, static_cast<double>(value) });
    });
}

void ProfilerWindow::impl::update_scroll_bars()
{
    QSize size = _widget->sizeHint();
    auto widget_size = _widget->size();
    if (size.width() <= _widget->width())
    {
        _scroll_barh->setVisible(false);
    }
    else
    {
        _scroll_barh->setRange(0, size.width() - widget_size.width());
        _scroll_barh->setVisible(true);
        _scroll_barh->setValue(_widget->scroll().x);
    }

    if (size.height() <= _widget->height())
    {
        _scroll_barv->setVisible(false);
    }
    else
    {
        _scroll_barv->setRange(0, size.height() - widget_size.height());
        _scroll_barv->setVisible(true);
        _scroll_barh->setValue(_widget->scroll().y);
    }
};

ProfilerWindow::ProfilerWindow(QWidget* parent)
    : QMainWindow(parent)
    , _impl(std::make_unique<impl>(this))
{
    setWindowTitle("Profiler Window");
    resize(800, 600);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto toolbar = addToolBar("Profiler Toolbar");

    QWidget* central_widget = new QWidget(this);
    QGridLayout* layout = new QGridLayout(central_widget);

    central_widget->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    _impl->setup();

    layout->addWidget(_impl->_widget, 0, 0);
    layout->addWidget(_impl->_scroll_barh, 1, 0);
    layout->addWidget(_impl->_scroll_barv, 0, 1);

    setCentralWidget(central_widget);

    QTimer* timer = new QTimer(this);
    connect(timer,
            &QTimer::timeout,
            this,
            [ this ]() { _impl->update_scroll_bars(); });

    _impl->update_scroll_bars();

    ui::MultiSpinBox<2, double>* zoomSpinBox =
        new ui::MultiSpinBox<2, double>(this);
    ui::MultiSpinBox<2, double>* scrollSpinBox =
        new ui::MultiSpinBox<2, double>(this);

    zoomSpinBox->setLabel("Zoom");
    zoomSpinBox->setRangeMin({ 0.00001, 0.00001 });
    zoomSpinBox->setRangeMax({ 100, 100 });
    zoomSpinBox->setValue(_impl->_widget->zoom());
    zoomSpinBox->setStep({ 0.01, 0.01 });
    zoomSpinBox->setSingleStep({ 1.0, 1.0 });
    zoomSpinBox->setPageStep({ 10, 10 });
    zoomSpinBox->setValue(_impl->_widget->zoom());
    zoomSpinBox->value_changed += [ this, widget = _impl->_widget ](auto value)
    {
        widget->setZoom(value);
        _impl->update_scroll_bars();
    };
    toolbar->addWidget(zoomSpinBox);
    // scrollSpinBox->setLabel("Scroll");
    // scrollSpinBox->setRangeMin({ -std::numeric_limits<double>::infinity(),
    //                              -std::numeric_limits<double>::infinity() });
    // scrollSpinBox->setRangeMax({ std::numeric_limits<double>::infinity(),
    //                              std::numeric_limits<double>::infinity() });
    // scrollSpinBox->setValue(profiler_widget->scroll());
    // scrollSpinBox->setStep({ 0.01, 0.01 });
    // scrollSpinBox->setSingleStep({ 1, 1 });
    // scrollSpinBox->setPageStep({ 4, 4 });
    // scrollSpinBox->setValue(profiler_widget->scroll());
    // scrollSpinBox->value_changed +=
    //     [ profiler_widget ](auto value) { profiler_widget->setScroll(value);
    //     };
    toolbar->addWidget(scrollSpinBox);
    QToolButton* visualizeFrameButton = new QToolButton(this);
    visualizeFrameButton->setText("Overall view");
    visualizeFrameButton->setCheckable(false);
    connect(visualizeFrameButton,
            &QToolButton::clicked,
            this,
            [ this, widget = _impl->_widget ]()
    {
        widget->reset();
        _impl->update_scroll_bars();
    });

    QToolButton* stopProfilingButton = new QToolButton(this);
    stopProfilingButton->setText("Stop");
    stopProfilingButton->setCheckable(false);
    connect(stopProfilingButton,
            &QToolButton::clicked,
            this,
            [ this, timer ]()
    {
        prof::stop();
        timer->stop();
        _impl->update_scroll_bars();
    });

    QToolButton* startProfilingButton = new QToolButton(this);
    startProfilingButton->setText("Start");
    startProfilingButton->setCheckable(false);
    connect(startProfilingButton,
            &QToolButton::clicked,
            this,
            [ timer ]()
    {
        prof::start();
        timer->start(100);
    });
    toolbar->addWidget(startProfilingButton);
    toolbar->addWidget(stopProfilingButton);
    toolbar->addWidget(visualizeFrameButton);
}

ProfilerWindow::~ProfilerWindow() = default;

void ProfilerWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    _impl->update_scroll_bars();
}
