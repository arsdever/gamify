#pragma once

#include <QTreeView>

namespace ui
{
class SceneViewWidget : public QTreeView
{
    Q_OBJECT
public:
    SceneViewWidget(QWidget* parent = nullptr);
    ~SceneViewWidget();

private:
    void initialize();

private:
    struct SceneViewWidgetPrivate;
    std::unique_ptr<SceneViewWidgetPrivate> _p;
};
} // namespace ui
