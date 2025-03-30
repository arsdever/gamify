#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

#include <qspdlog/qspdlog.hpp>
#include <spdlog/spdlog.h>

#include "editor_main_window.hpp"

EditorMainWindow::EditorMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    initialize();
}

EditorMainWindow::~EditorMainWindow()
{
    // Destructor implementation (if needed)
}

void EditorMainWindow::initialize()
{
    // Initialization code (if needed)
    setWindowTitle("Prefab Editor");

    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu("File");
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &EditorMainWindow::close);
    setMenuBar(menuBar);

    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    QSpdLog* loggerWidget = new QSpdLog(this);
    QDockWidget* loggerDock = new QDockWidget("Logger", this);
    loggerDock->setWidget(loggerWidget);
    addDockWidget(Qt::BottomDockWidgetArea, loggerDock);

    auto sink = loggerWidget->sink();
    spdlog::default_logger()->sinks().push_back(sink);
}
