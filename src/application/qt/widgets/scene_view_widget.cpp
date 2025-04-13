#include "scene_view_widget.hpp"

#include "application/qt/prefab_editor/game_context.hpp"

namespace ui
{
struct SceneViewWidget::SceneViewWidgetPrivate
{
};

SceneViewWidget::SceneViewWidget(QWidget* parent)
    : QTreeView(parent)
    , _p(std::make_unique<SceneViewWidgetPrivate>())
{
    initialize();
}

SceneViewWidget::~SceneViewWidget() = default;

void SceneViewWidget::initialize() { }
}; // namespace ui
