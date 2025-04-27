#include <QToolBar>

#include <application/qt/widgets/profiler_widget.hpp>

#include "profiler_window.hpp"

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

    toolbar->addAction("Zoom In",
                       this,
                       [ profiler_widget ]()
    { profiler_widget->setZoom(profiler_widget->zoom() * 1.1); });
    toolbar->addAction("Zoom Out",
                       this,
                       [ profiler_widget ]()
    { profiler_widget->setZoom(profiler_widget->zoom() * 0.9); });
}
