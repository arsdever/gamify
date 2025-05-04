#include <QToolBar>
#include <QToolButton>

#include <application/qt/widgets/profiler_widget.hpp>
#include <prof/profiler.hpp>

#include "profiler_window.hpp"

#include "application/qt/widgets/multi_spinbox.hpp"

ProfilerWindow::ProfilerWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Profiler Window");
    resize(800, 600);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto toolbar = addToolBar("Profiler Toolbar");

    auto profiler_widget = ProfilerWidget::create();
    setCentralWidget(profiler_widget);

    ui::MultiSpinBox<2, double>* zoomSpinBox =
        new ui::MultiSpinBox<2, double>(this);
    ui::MultiSpinBox<2, double>* scrollSpinBox =
        new ui::MultiSpinBox<2, double>(this);

    zoomSpinBox->setLabel("Zoom");
    zoomSpinBox->setRangeMin({ 0.00001, 0.00001 });
    zoomSpinBox->setRangeMax({ 100, 100 });
    zoomSpinBox->setValue(profiler_widget->zoom());
    zoomSpinBox->setStep({ 0.01, 0.01});
    zoomSpinBox->setSingleStep({ 1.0, 1.0 });
    zoomSpinBox->setPageStep({ 10, 10 });
    zoomSpinBox->setValue(profiler_widget->zoom());
    zoomSpinBox->value_changed +=
        [ profiler_widget ](auto value) { profiler_widget->setZoom(value); };
    toolbar->addWidget(zoomSpinBox);
    scrollSpinBox->setLabel("Scroll");
    scrollSpinBox->setRangeMin({ -std::numeric_limits<double>::infinity(),
                                 -std::numeric_limits<double>::infinity() });
    scrollSpinBox->setRangeMax({ std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::infinity() });
    scrollSpinBox->setValue(profiler_widget->scroll());
    scrollSpinBox->setStep({ 0.01, 0.01 });
    scrollSpinBox->setSingleStep({ 1, 1 });
    scrollSpinBox->setPageStep({ 4, 4 });
    scrollSpinBox->setValue(profiler_widget->scroll());
    scrollSpinBox->value_changed +=
        [ profiler_widget ](auto value) { profiler_widget->setScroll(value); };
    toolbar->addWidget(scrollSpinBox);
    QToolButton* visualizeFrameButton = new QToolButton(this);
    visualizeFrameButton->setText("Overall view");
    visualizeFrameButton->setCheckable(false);
    connect(visualizeFrameButton,
            &QToolButton::clicked,
            this,
            [ profiler_widget ]() { profiler_widget->reset(); });

    QToolButton* stopProfilingButton = new QToolButton(this);
    stopProfilingButton->setText("Stop");
    stopProfilingButton->setCheckable(false);
    connect(stopProfilingButton,
            &QToolButton::clicked,
            this,
            []() { prof::stop(); });

    QToolButton* startProfilingButton = new QToolButton(this);
    startProfilingButton->setText("Start");
    startProfilingButton->setCheckable(false);
    connect(startProfilingButton,
            &QToolButton::clicked,
            this,
            []() { prof::start(); });
    toolbar->addWidget(startProfilingButton);
    toolbar->addWidget(stopProfilingButton);
    toolbar->addWidget(visualizeFrameButton);
}
