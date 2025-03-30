#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

#include <common/logging.hpp>
#include <qspdlog/qspdlog.hpp>

#include "editor_main_window.hpp"

#include "game_context.hpp"

inline logger log() { return get_logger("editor"); }

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

    QMenu* newMenu = fileMenu->addMenu("New");
    QAction* newGameObject = newMenu->addAction("Empty Game Object");
    QAction* newCamera = newMenu->addAction("Camera");
    QAction* newCube = newMenu->addAction("Cube");

    QAction* exitAction = fileMenu->addAction("Exit");

    connect(newGameObject,
            &QAction::triggered,
            [] { game_context::create_empty_game_object(); });
    connect(
        newCamera, &QAction::triggered, [] { game_context::create_camera(); });
    connect(newCube, &QAction::triggered, [] { game_context::create_cube(); });
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
