#pragma once

#include <QScrollArea>

class game_object;

namespace ui
{
class InspectorWidget : public QScrollArea
{
    Q_OBJECT
public:
    InspectorWidget(QWidget* parent = nullptr);
    ~InspectorWidget();

    void setInspectingObject(std::shared_ptr<game_object> t);
    void resetInspector();

    QSize sizeHint() const override;

private:
    struct InspectorWidgetPrivate;
    std::unique_ptr<InspectorWidgetPrivate> _p;
};
} // namespace ui
