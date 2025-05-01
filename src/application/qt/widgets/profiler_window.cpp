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

    profiler_widget->setZoom({ 0.3, 0.0001 });

    ui::MultiSpinBox<2, double>* zoomSpinBox =
        new ui::MultiSpinBox<2, double>(this);
    // ui::MultiSpinBox<2, double>* scrollSpinBox =
    //     new ui::MultiSpinBox<2, double>(this);

    zoomSpinBox->setLabel("Zoom");
    zoomSpinBox->setRangeMin({ 0.00001, 0.00001 });
    zoomSpinBox->setRangeMax({ 100, 100 });
    zoomSpinBox->setValue(profiler_widget->zoom());
    zoomSpinBox->setStep({ 0.0000001, 0.0000001 });
    zoomSpinBox->setSingleStep({ 0.0001, 0.0001 });
    zoomSpinBox->setPageStep({ 0.1, 0.1 });
    zoomSpinBox->value_changed +=
        [ profiler_widget ](auto value) { profiler_widget->setZoom(value); };
    toolbar->addWidget(zoomSpinBox);
    QToolButton* visualizeFrameButton = new QToolButton(this);
    visualizeFrameButton->setText("Visualize Frame");
    visualizeFrameButton->setCheckable(false);
    connect(visualizeFrameButton,
            &QToolButton::clicked,
            this,
            [ profiler_widget ]() { profiler_widget->snapshot(); });

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
