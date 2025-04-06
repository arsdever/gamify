#pragma once

#include <QWidget>

namespace ui
{
// Based on https://stackoverflow.com/a/37119983/10185183
class CollapsibleWidget : public QWidget
{
    Q_OBJECT

public:
    CollapsibleWidget(const QString& title = "",
                      const int animationDuration = 100,
                      QWidget* parent = 0);
    ~CollapsibleWidget() override;
    void setContentLayout(QLayout* contentLayout);

private:
    struct CollapsibleWidgetPrivate;
    std::unique_ptr<CollapsibleWidgetPrivate> _p;
};
} // namespace ui
