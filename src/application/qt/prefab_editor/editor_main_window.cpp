#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTreeView>

#include <application/qt/widgets/inspector_widget.hpp>
#include <application/qt/widgets/scene_model.hpp>
#include <application/qt/widgets/scene_view_widget.hpp>
#include <common/logging.hpp>
#include <project/components/transform.hpp>
#include <project/game_object.hpp>
#include <project/scene.hpp>
#include <qspdlog/qspdlog.hpp>

#include "editor_main_window.hpp"

#include "game_context.hpp"

inline logger log() { return get_logger("editor"); }

struct EditorMainWindow::impl
{
    void initialize();

    void setupDockWidgets();
    void setupInspectorWidget();
    void setupSceneView();
    void setupLoggerWidget();
    void setupProfilerWidget();

    void setupStatusBar(QStatusBar* statusBar);

    void setupMenuBar(QMenuBar* menuBar);
    void setupNewMenu(QMenu* newMenu);
    void setupViewMenu(QMenu* viewMenu);

    EditorMainWindow* _self = nullptr;
    QDockWidget* _inspectorWidget = nullptr;
    QDockWidget* _sceneViewWidget = nullptr;
    QDockWidget* _loggerWidget = nullptr;
    QDockWidget* _profilerWidget = nullptr;
};

EditorMainWindow::EditorMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _p(std::make_unique<impl>(this))
{
    _p->initialize();
}

EditorMainWindow::~EditorMainWindow()
{
    // Destructor implementation (if needed)
}

void EditorMainWindow::impl::initialize()
{
    // Initialization code (if needed)
    _self->setWindowTitle("Prefab Editor");

    setupDockWidgets();

    QMenuBar* menuBar = new QMenuBar(_self);
    setupMenuBar(menuBar);

    QStatusBar* statusBar = new QStatusBar(_self);
    setupStatusBar(statusBar);

    game_context::on_object_selected += [ this ](auto obj)
    {
        auto sceneView =
            qobject_cast<ui::SceneViewWidget*>(_sceneViewWidget->widget());
        auto inspectorView =
            qobject_cast<ui::InspectorWidget*>(_inspectorWidget->widget());
        auto model = qobject_cast<ui::SceneModel*>(sceneView->model());

        auto gobj = std::static_pointer_cast<game_object>(obj);
        inspectorView->setInspectingObject(gobj);
        log()->debug("Selected object: {}", gobj->get_name());
        auto idx = model->indexOf(gobj);
        sceneView->setCurrentIndex(idx);
        inspectorView->setInspectingObject(gobj);
    };
}

void EditorMainWindow::impl::setupMenuBar(QMenuBar* menuBar)
{
    _self->setMenuBar(menuBar);

    setupNewMenu(menuBar->addMenu("New"));
    setupViewMenu(menuBar->addMenu("View"));

    QAction* exitAction = menuBar->addAction("Exit");
    _self->connect(
        exitAction, &QAction::triggered, _self, &EditorMainWindow::close);
}

void EditorMainWindow::impl::setupNewMenu(QMenu* newMenu)
{
    QAction* newGameObject = newMenu->addAction("Empty Game Object");
    newMenu->addSeparator();
    QAction* newCamera = newMenu->addAction("Camera");
    QAction* newLight = newMenu->addAction("Light");
    newMenu->addSeparator();
    QAction* newCube = newMenu->addAction("Cube");

    connect(newGameObject,
            &QAction::triggered,
            [] { game_context::create_empty_game_object(); });
    connect(
        newCamera, &QAction::triggered, [] { game_context::create_camera(); });
    connect(newCube, &QAction::triggered, [] { game_context::create_cube(); });
    connect(
        newLight, &QAction::triggered, [] { game_context::create_light(); });
}

void EditorMainWindow::impl::setupViewMenu(QMenu* viewMenu)
{
    QAction* inspectorViewAction = viewMenu->addAction("Inspector");
    QAction* sceneViewViewAction = viewMenu->addAction("Scene View");
    QAction* loggerViewAction = viewMenu->addAction("Logger");
    QAction* profilerViewAction = viewMenu->addAction("Profiler");

    inspectorViewAction->setCheckable(true);
    sceneViewViewAction->setCheckable(true);
    loggerViewAction->setCheckable(true);
    profilerViewAction->setCheckable(true);

    inspectorViewAction->setChecked(_inspectorWidget->isVisible());
    sceneViewViewAction->setChecked(_sceneViewWidget->isVisible());
    loggerViewAction->setChecked(_loggerWidget->isVisible());
    profilerViewAction->setChecked(_profilerWidget->isVisible());

    connect(inspectorViewAction,
            &QAction::triggered,
            [ this ](bool checked) { _inspectorWidget->setVisible(checked); });

    connect(sceneViewViewAction,
            &QAction::triggered,
            [ this ](bool checked) { _sceneViewWidget->setVisible(checked); });

    connect(loggerViewAction,
            &QAction::triggered,
            [ this ](bool checked) { _loggerWidget->setVisible(checked); });

    connect(profilerViewAction,
            &QAction::triggered,
            [ this ](bool checked) { _profilerWidget->setVisible(checked); });

    connect(_inspectorWidget,
            &QDockWidget::visibilityChanged,
            [ inspectorViewAction ](bool visible)
    {
        auto b = inspectorViewAction->blockSignals(true);
        inspectorViewAction->setChecked(visible);
        inspectorViewAction->blockSignals(b);
    });

    connect(_sceneViewWidget,
            &QDockWidget::visibilityChanged,
            [ sceneViewViewAction ](bool visible)
    {
        auto b = sceneViewViewAction->blockSignals(true);
        sceneViewViewAction->setChecked(visible);
        sceneViewViewAction->blockSignals(b);
    });

    connect(_loggerWidget,
            &QDockWidget::visibilityChanged,
            [ loggerViewAction ](bool visible)
    {
        auto b = loggerViewAction->blockSignals(true);
        loggerViewAction->setChecked(visible);
        loggerViewAction->blockSignals(b);
    });

    connect(_profilerWidget,
            &QDockWidget::visibilityChanged,
            [ profilerViewAction ](bool visible)
    {
        auto b = profilerViewAction->blockSignals(true);
        profilerViewAction->setChecked(visible);
        profilerViewAction->blockSignals(b);
    });
}

void EditorMainWindow::impl::setupStatusBar(QStatusBar* statusBar)
{
    _self->setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

void EditorMainWindow::impl::setupDockWidgets()
{
    setupInspectorWidget();
    setupSceneView();
    setupLoggerWidget();
    setupProfilerWidget();
}

void EditorMainWindow::impl::setupInspectorWidget()
{
    auto inspectorWidget = new ui::InspectorWidget(_self);
    QDockWidget* inspectorDock = new QDockWidget("Inspector", _self);
    _inspectorWidget = inspectorDock;
    inspectorDock->setWidget(inspectorWidget);
    _self->addDockWidget(Qt::RightDockWidgetArea, inspectorDock);
    inspectorDock->setAttribute(Qt::WA_DeleteOnClose, false);
}

void EditorMainWindow::impl::setupSceneView()
{
    auto sceneViewWidget = new ui::SceneViewWidget(_self);
    QDockWidget* sceneDock = new QDockWidget("Scene", _self);
    _sceneViewWidget = sceneDock;
    sceneDock->setWidget(sceneViewWidget);
    _self->addDockWidget(Qt::LeftDockWidgetArea, sceneDock);
    sceneDock->setAttribute(Qt::WA_DeleteOnClose, false);

    auto sceneModel = new ui::SceneModel(scene::get_active_scene(), _self);
    sceneViewWidget->setModel(sceneModel);

    sceneViewWidget->connect(sceneViewWidget,
                             &QAbstractItemView::clicked,
                             [](const QModelIndex& index)
    {
        auto activated_game_object =
            index.data(Qt::UserRole).value<std::shared_ptr<game_object>>();
        std::vector<std::shared_ptr<game_object>> objects {
            activated_game_object
        };
        game_context::set_object_selection(objects);
    });
}

void EditorMainWindow::impl::setupLoggerWidget()
{
    auto loggerWidget = new QSpdLog(_self);
    QDockWidget* loggerDock = new QDockWidget("Logger", _self);
    _loggerWidget = loggerDock;
    loggerDock->setWidget(loggerWidget);
    _self->addDockWidget(Qt::BottomDockWidgetArea, loggerDock);
    loggerDock->setAttribute(Qt::WA_DeleteOnClose, false);

    auto sink = loggerWidget->sink();
    spdlog::default_logger()->sinks().push_back(sink);
    loggerWidget->setAutoScrollPolicy(
        AutoScrollPolicy::AutoScrollPolicyEnabledIfBottom);
}

void EditorMainWindow::impl::setupProfilerWidget()
{
    // TODO: Implement profiler widget
    // auto profilerWidget = new ProfilerWidget(_self);
    QDockWidget* profilerDock = new QDockWidget("Profiler", _self);
    _profilerWidget = profilerDock;
    // profilerDock->setWidget(profilerWidget);
    _self->addDockWidget(Qt::BottomDockWidgetArea, profilerDock);
    profilerDock->setAttribute(Qt::WA_DeleteOnClose, false);
}
